/*
Arbolog control programm 

Arduino SAMD Board 1.8.14

global defines in arbolog.h

PIN definitions:
https://github.com/arduino/ArduinoCore-samd/blob/master/variants/mkrzero/variant.cpp

Configuration options:
  Devicename DNS-<customer>-<sensor>
  Serveradress Serveradress where data are sent to
  PIN Pin for the Network (not WIFI)

  to be set using the serialUSB 
    #setdevice <name>
    #setserver <name>
    #setpin <pin>
    afterwards #update

  New: Configuration vis SD card contents
    SD card needs a FAT32 file system
    the first volume is readable
    configuration file: arbolog.cfg



Akku: us18650vtc4/INR18650-35E

*/

#include "arbolog.h"
#include <stdio.h>
#include <string.h>

#include <RTCZero.h>

#include <SPI.h>
/* Device is
 * 128KB: SRAM_23LCV1024
 */
#include <SRAM_23LC.h>
// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include <FlashStorage_SAMD.h>
#include "adrastea.h"
#include "arbologconfig.h"

#include "sdcard.h"
#include "wsenisds.h"
#include "cli.h"

#include "SAMDTimerInterrupt.h"
#include "SAMD_ISR_Timer.h" 

// Use 0-2. Larger for more debugging messages
#define FLASH_DEBUG       0

#include <ArduinoJson.h>

StaticJsonDocument<1024> JSONConfig;

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
//#include "ISR_Timer_Generic.h"

//#include <SimpleTimer.h>      

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

String from_usb = "";

Adrastea adrastea;
ArbologConfig arbologConfig;
RTCZero rtc;
sdcard SDCard;
WSENISDS wsen;
SRAM_23LC SRAM(&SPI, SRAM_SELECT_PIN, SRAM_23LCV1024);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define SELECTED_TIMER      TIMER_TC3
//SAMDTimer 
//SAMD_ISR_Timer 
SAMD_ISRTimer ITimer;

#define TIMER_INTERVAL_MS 1000
volatile int measurements = 0;
volatile int vibration = 0;
volatile bool btnpress = false;

void TimerHandler(){
  Serial.print("ITimer: millis() = "); 
  Serial.print(millis());
  Serial.print(" Number: ");
  Serial.println(measurements);
  measurements++;
}


void setup()
{
  SerialUSB.begin(115200);
  Serial1.begin(57600);
  while(!Serial);

  SPI.begin();
  SRAM.begin();
  SDCard.begin();

  pixels.begin();
//  pixels.setPixelColor(0, pixels.Color(125,125,0));
//  pixels.setPixelColor(1, pixels.Color(0,125,255));
//  pixels.show();

  Serial.println(BOARD_NAME);
  Serial.println(FLASH_STORAGE_SAMD_VERSION);

  Serial.print("EEPROM length: ");
  Serial.println(EEPROM.length());
  
  pixels.setPixelColor(1, pixels.Color(0,0,0));
  pixels.setPixelColor(0, pixels.Color(0,0,0));
  pixels.show();
  arbologConfig.begin();
  Serial.println(arbologConfig.getPasscode("james-tiberius-kirk"));
  rtc.begin();
  rtc.attachInterrupt(alarm);

// Contents of the rtc after reset
  SerialUSB.print(rtc.getDay());
  SerialUSB.print(".");
  SerialUSB.print(rtc.getMonth());
  SerialUSB.print(".");
  SerialUSB.println(rtc.getYear()+2000);

  SerialUSB.print(rtc.getHours());
  SerialUSB.print(":");
  SerialUSB.print(rtc.getMinutes());
  SerialUSB.print(":");
  SerialUSB.println(rtc.getSeconds());

  wsen.setCSPin(CSWSENS_PIN);
  wsen.begin();
  wsen.setAccelerationRate( RATE6_66kHz ); 
  wsen.setAccelerationRange(2);
  wsen.setGyroscopeRate( RATE6_66kHz ); 
  wsen.setGyroscopeRange(1);

  Serial.println(wsen.getDeviceID(), HEX);
  if (!digitalRead(BUTTON_PIN)){
    SerialUSB.println("Button press during setup");
    wsen.calibrateGyroscope();
    SerialUSB.print("Gyroscope calibrated:");
    SerialUSB.print(wsen.getGyroOffsetX());
    SerialUSB.print(",");
    SerialUSB.print(wsen.getGyroOffsetY());
    SerialUSB.print(",");
    SerialUSB.println(wsen.getGyroOffsetZ());
    wsen.calibrateAcc();
    SerialUSB.print("ACC calibrated:");
    SerialUSB.print(wsen.getAccOffsetX());
    SerialUSB.print(",");
    SerialUSB.print(wsen.getAccOffsetY());
    SerialUSB.print(",");
    SerialUSB.println(wsen.getAccOffsetZ());
    arbologConfig.updateEEPROM();
//      while(!digitalRead(BUTTON_PIN)){
//     }
    }
    SerialUSB.print("Gyroscope offset:");
    SerialUSB.print(wsen.getGyroOffsetX());
    SerialUSB.print(",");
    SerialUSB.print(wsen.getGyroOffsetY());
    SerialUSB.print(",");
    SerialUSB.println(wsen.getGyroOffsetZ());
  #ifdef WITHADRASTEA
    adrastea.setSDPin(ADASTEASD_PIN);
    adrastea.shutdown();
    adrastea.wakeup();
    adrastea.setUart(&Serial1);
    Serial.print("Serial:");
    Serial.println(adrastea.getSerialNumber());
    // 01515 5562112
    adrastea.setPIN(arbologConfig.getPin());
    // delay(150);
    adrastea.readSatelliteData();
    
    String tmp = adrastea.getDate();
    if (tmp.length()==10){
      int day, month, year;
      sscanf(tmp.c_str(), "%d/%d/%d", &day,&month,&year);
      rtc.setDate(day,month,year-2000);
    }
    tmp = adrastea.getTime();
    if (tmp.length()==8){
      int hour, minute, second;
      sscanf(tmp.c_str(), "%d:%d:%d", &hour,&minute,&second);
      rtc.setTime(hour,minute,second);
    }
  #endif

  #ifdef WLAN
    
/*    SerialUSB.println("obtaining date");
    String response=sendData("print date",3000,true);
    SerialUSB.print("print date:");
    SerialUSB.println(response);
    setRtcFromString(response);*/
    pinMode(MODULE_WAKEUP, OUTPUT);
    // Enable WLAN module
    digitalWrite(MODULE_WAKEUP, LOW);
    pinMode(MODULE_POWER, OUTPUT);
    digitalWrite(MODULE_POWER, HIGH);
  #endif

    ITimer.setTimer(TIMER_INTERVAL_MS, TimerHandler, 10);
    ITimer.run();
    SerialUSB.println("Timer Set");
    attachInterrupt(digitalPinToInterrupt(VIBRATION_PIN), vib, RISING);
    pixels.clear(); // Set all pixel colors to 'off'
    pixels.show();
}
 
String from_Serial1 = "";

bool blink = false;
unsigned long blinkinterval = 0;

void loop()
{
  // process the serial lines 
  processSerial1();
  processSerialUSB();
  //
  /*
  if (rtc.getYear() < 23 || rtc.getYear() > 35){
    SerialUSB.print(rtc.getYear());
    
    String response=sendData("print date",3000,true);
    SerialUSB.print("print date:");
    SerialUSB.println(response);
    setRtcFromString(response);
    delay(1000);
  }
  */
  // process interrupt events 
  if (btnpress){
    btnpress = false;
    Serial.println("Btn press");
  }
  if (vibration ) { //|| arbologConfig.isSampling()){
/*    vibration++;
    if (vibration > 1000) */
      vibration = 0;
    processVibration();
    delay(100);
  }
  if (millis() - blinkinterval > 300){
    blinkinterval = millis();
    if (blink){
      blink = false;
      digitalWrite(MODULE_WAKEUP, LOW);
    }
    else{
      blink = true;
      digitalWrite(MODULE_WAKEUP, HIGH);
    }
  }
}

void vib(){
  vibration = 1;
}

void bpr(){
  btnpress = true;
}

void alarm(){
//  USBDevice.attach();
//   while(!SerialUSB);
  // Serial USB is blazing fast, you might miss the messages
  delay(1000);
  SerialUSB.println("Alarm");
  printTime();
//  rtc.standbyMode();
}

void printTime(){
  SerialUSB.print(rtc.getHours()); 
  SerialUSB.print(":"); 
  SerialUSB.print(rtc.getMinutes()); 
  SerialUSB.print(":"); 
  SerialUSB.println(rtc.getSeconds());
}

void processSerial1(){
  while (Serial1.available() > 0)
  {
      int c = -1; 
      c = Serial1.read();
      if (c != '\n' && c != '\r')
      {
          from_Serial1 += (char)c;
      }
      else{
        cli(from_Serial1, Serial1, true);
        from_Serial1 = "";
      }        
      yield();
  }
}

void processSerialUSB(){
    while (SerialUSB.available() > 0)
    {
#ifdef MODE_1A
        from_usb = SerialUSB.readStringUntil('\r');
        if (!from_usb.equals("")){
          if(from_usb.startsWith("#")){
            cli(from_usb, SerialUSB, false);
          }
          else{
            sendData(from_usb, 0, DEBUG);
            from_usb = "";
        }
    }
#else
        Serial1.write(SerialUSB.read());
        yield();
#endif

  }
}

static char tenthsecond = '0';
static char lastsec = '0';

void processVibration(){
  if(!arbologConfig.isSampling()){
    pixels.setPixelColor(0, pixels.Color(125,125,0));
    pixels.show();
  }

  arbologConfig.incVibration();
  Serial.print("VIBRATION~~~~~~~~~~~~~~~");
  Serial.println(arbologConfig.getVibration());
  if (!digitalRead(BUTTON_PIN)){
    Serial.println(" with button press");
    while (!digitalRead(BUTTON_PIN)){
      pixels.setPixelColor(1, pixels.Color(0,255,0));
      pixels.show();
      delay(500);
      pixels.setPixelColor(1, pixels.Color(255,0,0));
      pixels.show();
      delay(500);
    }
  }
  Serial.println("Device ID must be 6A");
  Serial.println(wsen.getDeviceID(), HEX);
  Serial.print("Status[WSEN]: ");
  Serial.println(wsen.readRegister8(STATUS_REG), HEX);
  Serial.print("Temp[float]: ");
  Serial.println(wsen.getDegreeCentigrade());
  wsen.setGyroscopeRate( RATE6_66kHz ); 
  wsen.setGyroscopeRange(1);

  Serial.println(wsen.readRegister8(CTRL2_G));
  wsen.setAccelerationRate( RATE6_66kHz );
  for (int i=0; i< 10; i++){
    
    String sendString="{\"sensor\":\"";
    sendString += arbologConfig.getDevicename();
    sendString += "\",\"date\":\"";
    sendString = sendString + getRTCDate();
    sendString += "\",\"temp\":";
    sendString = sendString + ((int) floor(arbologConfig.getTemperature()*10));
    sendString += ",\"voltage\":";
    sendString = sendString + ((int) floor(arbologConfig.getAkkuVoltage()*10));
    sendString += ",\"data\":[";
    for (int measures = 0; measures < 10; measures++){
      long measure = millis();  
      float Ax = wsen.readAccX();
      float Ay = wsen.readAccY();
      float Az = wsen.readAccZ();
  //    Serial.print("Accel X: ");
  //    Serial.print(Ax,5);
  //    Serial.print(" Y: ");
  //    Serial.print(Ay,5);
  //    Serial.print(" Z: ");
  //    Serial.println(Az,5);
      float Gx = wsen.readGyroX();
      float Gy = wsen.readGyroY();
      float Gz = wsen.readGyroZ();
  //    Serial.print("Gyro X: ");
  //    Serial.print(Gx,5);
  //    Serial.print(" Y: ");
  //    Serial.print(Gy,5);
  //    Serial.print(" Z: ");
  //    Serial.println(Gz,5);
      
      //sendString = sendString + ("{\"time\":\"");
      sendString = sendString + "\"";
      String time = getRTCTime();
      if (lastsec != time[5]){
        lastsec = time[5];
        tenthsecond = '0';
      }    
//      sendString = sendString + time + tenthsecond++;
//     sendString = sendString + (",");
      
      if (tenthsecond > '9')
        tenthsecond = '0';
        sendString = sendString + getSecondsOfDay();
        sendString = sendString + tenthsecond++;
      sendString = sendString + ",";
      
//      sendString = sendString + measures;
//      sendString = sendString + (",");
      sendString = sendString + ((int) floor(Ax));
      sendString = sendString + (",");
      sendString = sendString + ((int) floor(Ay));
      sendString = sendString + (",");
      sendString = sendString + ((int) floor(Az));
      sendString = sendString + (",");
      sendString = sendString + ((int) floor(Gx));
      sendString = sendString + (",");
      sendString = sendString + ((int) floor(Gy));
      sendString = sendString + (",");
      sendString = sendString + ((int) floor(Gz));
      double roll = 0.00, pitch = 0.00;       
      //Roll & Pitch are the angles which rotate by the axis X and y
        roll = atan2(Az, -Ay) * 57.3;
        pitch = atan2((- Ax) , sqrt(Ay * Ay + Az * Az)) * 57.3;
      sendString = sendString + (",");
      sendString = sendString + ((int) floor(roll*1000));
      sendString = sendString + (",");
      sendString = sendString + ((int) floor(pitch*1000));
      
      //      double angle = sqrt(roll*roll + pitch*pitch);
      //sendString = sendString + (",");
      //sendString = sendString + ((int) floor(angle*10));
  //    sendString = sendString + "\"]";
      sendString = sendString + "\"";
      if(measures != 9)
        sendString = sendString + ",";
      //sendString = sendString + ("}");
      //delay(10);
  /*    if (sendString.indexOf("send") == -1){
        Serial.println("CATCHED");
        Serial.println(sendString);
        Serial.print(" Y: ");
        Serial.print(Gy,5);
        Serial.print(" Z: ");
        Serial.println(Gz,5);
      }
    */
      while(millis() - measure < 100);
    }
    sendString = sendString + "]}";
    Serial1.print("send ");
    Serial1.println(sendString);
    Serial.println(sendString);
    Serial.println(sendString.length());
    sendString = "";    
  }

  wsen.setGyroscopeRate( POWERDOWN );
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();
}


