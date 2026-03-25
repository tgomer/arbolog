#ifndef __CLI_H__
#define __CLI_H__

#include "arbologconfig.h"

extern ArbologConfig arbologConfig;
extern WSENISDS wsen;
extern RTCZero rtc;


String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    if (command.equals("1A") || command.equals("1a"))
    {
        SerialUSB.println();
        SerialUSB.println("Get a 1A, sending Ctrl-Z");
        
        //Serial1.write(0x1A);
        Serial1.write(26);
//        Serial1.println();
//        return "";
    }
    if (command.equals("1B") || command.equals("1b"))
    {
      // adrastea.sendSMS("+491711487271", "Hallo Tobias, \r\nwie geht es?");
    }
/*    if (command.equals("1C") || command.equals("1c"))
    {
      Adrastea_SMSHeader_t *header = adrastea.getSMSHeaders();
      int index=0;
      Serial.println("Headers");
      while (header[index].index != 0){
        Serial.print(header[index].index, DEC);
        Serial.print(": ");
        Serial.print(header[index].status);
        Serial.print(" ");
        Serial.print(header[index].number);
        Serial.print(" ");
        Serial.print(header[index].date);
        Serial.print(" ");
        Serial.println(header[index].time);
        index++;
      } 
    } */
    if (command.startsWith("1R") || command.startsWith("1r")){
/*      int index;
      if (sscanf(command.c_str(), "1R%d", &index) == 1){
        String message = adrastea.readSMS(index);
        Serial.println("Read SMS");
        Serial.println(message);
      } */
    }
    else
    {
        Serial1.println(command);
    }
 
    unsigned long int time = millis();
    while ((time + timeout) > millis())
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            response += c;
        }
    }
    if (debug)
    {
        SerialUSB.println("debug: " + response);
    }
    return response;
}

void setRtcFromString(String datetime){
  SerialUSB.print("Setting RTC to: ");
  SerialUSB.println(datetime);
  int hour, minute, second;
  int day, month, year;
  sscanf(datetime.c_str(), "%d/%d/%d %d:%d:%d", &day,&month,&year, &hour,&minute,&second);
  rtc.setDate(day,month,year-2000);
  rtc.setTime(hour,minute,second);
}

String getRTCDate(){
  char retval[25];
  sprintf(retval, "%04d%02d%02d",  rtc.getYear()+2000, rtc.getMonth(), rtc.getDay());
  return retval;
}


String getRTCTime(){
  char retval[25];
  sprintf(retval, "%02d%02d%02d",  rtc.getHours(), rtc.getMinutes(), rtc.getSeconds() );
  return retval;
}

int getSecondsOfDay(){
  int retval =  rtc.getHours() * 3600;
  retval += rtc.getMinutes() * 60;
  retval += rtc.getSeconds();
  return retval;
}

String getRTCDateTime(){
  char retval[25];
  sprintf(retval, "%04d%02d%02d %02d%02d%02d",  rtc.getYear()+2000, rtc.getMonth(), rtc.getDay(), 
              rtc.getHours(), rtc.getMinutes(), rtc.getSeconds() );
  return retval;
}

void cli(const String & from_Serial, Stream& port, boolean replyCommand){
  Serial.println(from_Serial);
    if (from_Serial.startsWith("Unknown")){
      Serial.println(from_Serial);
      return;
    }
    if (from_Serial.startsWith("#echo")){
      port.print(from_Serial);
    }
    else if (from_Serial.startsWith("#getdate")){
      if(replyCommand) 
        port.print("#setdate ");
      port.println(getRTCDateTime());
    }
    else if (from_Serial.startsWith("#setdate")){
      Serial.println(from_Serial);
      setRtcFromString(from_Serial.c_str()+9);
      SerialUSB.println(getRTCDateTime());
    }
    else if (from_Serial.startsWith("#getvoltage")){
      port.println(arbologConfig.getAkkuVoltage());
    }
    else if (from_Serial.startsWith("#start")){
//      String response=sendData("print date",3000,true);
//      SerialUSB.print("print date:");
//      SerialUSB.println(response);
//      setRtcFromString(response);

      arbologConfig.startSampling();
      port.print("started");
    }
    else if (from_Serial.startsWith("#stop")){
      arbologConfig.stopSampling();
      port.print("stopped");
    }
    else if (from_Serial.startsWith("#gettemp")){
      float temperature = arbologConfig.getTemperature(); 
      port.println( temperature );
    }
    else if (from_Serial.startsWith("#getaccraw")){
      int16_t x = wsen.readAccXRaw();
      int16_t y = wsen.readAccYRaw();
      int16_t z = wsen.readAccZRaw();
      port.print( x );
      port.print( " " );
      port.print( y );
      port.print( " " );
      port.println( z );
    }
    else if (from_Serial.startsWith("#getgyroraw")){
      int16_t x = wsen.readGyroXRaw();
      int16_t y = wsen.readGyroYRaw();
      int16_t z = wsen.readGyroZRaw();
      port.print( x );
      port.print( " " );
      port.print( y );
      port.print( " " );
      port.println( z );
    }
    else if (from_Serial.startsWith("#getgyrooffset")){
      int16_t x = wsen.getGyroOffsetX();
      int16_t y = wsen.getGyroOffsetY();
      int16_t z = wsen.getGyroOffsetZ();
      port.print( x );
      port.print( " " );
      port.print( y );
      port.print( " " );
      port.println( z );
    }
    else if (from_Serial.startsWith("#getaccfloat")){
      float x = wsen.readAccX();
      float y = wsen.readAccY();
      float z = wsen.readAccZ();
      port.print( x );
      port.print( " " );
      port.print( y );
      port.print( " " );
      port.println( z );
    }
    else if (from_Serial.startsWith("#getgyrofloat")){
      float x = wsen.readGyroX();
      float y = wsen.readGyroY();
      float z = wsen.readGyroZ();
      port.print( x );
      port.print( " " );
      port.print( y );
      port.print( " " );
      port.println( z );
    }
    else if (from_Serial.startsWith("#getaccrange")){
      int16_t x = wsen.getAccelerationRange();
      port.println( x );
    }
    else if (from_Serial.startsWith("#setaccrange")){
      int16_t x;
      sscanf(from_Serial.c_str(), "#setaccrange %d", &x);
      wsen.setAccelerationRange(x);
      x = wsen.getAccelerationRange();
      port.println( x );
    }
    else if (from_Serial.startsWith("#getgyrorange")){
      int16_t x = wsen.getGyroscopeRange();
      port.println( x );
    }
    else if (from_Serial.startsWith("#setgyrorange")){
      int16_t x;
      sscanf(from_Serial.c_str(), "#setgyrorange %d", &x);
      wsen.setGyroscopeRange(x);
      x = wsen.getGyroscopeRange();
      port.println( x );
    }
    else if (from_Serial.startsWith("#getgyrooffset")){
      port.print( wsen.getGyroOffsetX());
      port.print(" ");
      port.print( wsen.getGyroOffsetY());
      port.print(" ");
      port.println( wsen.getGyroOffsetZ());
    }
    else if (from_Serial.startsWith("#setGyroscopeRate")){
      if (from_Serial.indexOf("POWERDOWN") != -1)
        wsen.setGyroscopeRate(POWERDOWN);              
      else if (from_Serial.indexOf("RATE12_5Hz") != -1)
        wsen.setGyroscopeRate(RATE12_5Hz);              
      else if (from_Serial.indexOf("RATE26Hz") != -1)
        wsen.setGyroscopeRate(RATE26Hz);
      else if (from_Serial.indexOf("RATE52Hz") != -1)
        wsen.setGyroscopeRate(RATE52Hz);
      else if (from_Serial.indexOf("RATE104Hz") != -1)
        wsen.setGyroscopeRate(RATE104Hz);
      else if (from_Serial.indexOf("RATE208Hz") != -1)
        wsen.setGyroscopeRate(RATE208Hz);
      else if (from_Serial.indexOf("RATE416Hz") != -1)
        wsen.setGyroscopeRate(RATE416Hz);
      else if (from_Serial.indexOf("RATE833Hz") != -1)
        wsen.setGyroscopeRate(RATE833Hz);
      else if (from_Serial.indexOf("RATE1_66kHz") != -1)
        wsen.setGyroscopeRate(RATE1_66kHz);
      else if (from_Serial.indexOf("RATE3_33kHz") != -1)
        wsen.setGyroscopeRate(RATE3_33kHz);
      else if (from_Serial.indexOf("RATE6_66kHz") != -1)
        wsen.setGyroscopeRate(RATE6_66kHz);
      else port.println("Invalid input");
      byte x = wsen.getGyroscopeRate();
      port.println( x, BIN );
    }
    else if (from_Serial.startsWith("#getgyrorate")){
      byte x = wsen.getGyroscopeRate();
      port.println( x, BIN );
    }
    else if (from_Serial.startsWith("#readregister")){
      byte x;
      sscanf(from_Serial.c_str(), "#readregister %x", &x);
      x = wsen.readRegister8(x);
      port.print(x,HEX);
      port.print(" ");
      port.println(x,BIN);
    }
    else if (from_Serial.startsWith("#writeregister")){
      int x, y;
      sscanf(from_Serial.c_str(), "#writeregister %x %x", &x, &y);
      port.print("write: ");
      port.print(x, HEX); 
      port.print("to: ");
      port.println(y, HEX); 
      wsen.writeRegister8(x,y);
      port.print(x,HEX);
      port.print(" ");
      port.println(y,HEX);
    }
    else if (from_Serial.startsWith("#update")){
      arbologConfig.updateEEPROM();
      port.println( "EEPROM updated." );
    }
    else if (from_Serial.startsWith("#dump")){
      arbologConfig.dumpEEPROM();
    }
    else if (from_Serial.startsWith("#getpasscode")){
      int pos = from_Serial.indexOf(' ');
      port.print(from_Serial);
      port.print(" ");
      port.println(arbologConfig.getPasscode(from_Serial.substring(pos+1).c_str()).c_str());
    }
    else if (from_Serial.startsWith("#gethostname")){
      port.print(from_Serial);
      port.print(" ");
      port.println(arbologConfig.getDevicename());
    }
    else if (from_Serial.startsWith("#setdevice")){
      int pos = from_Serial.indexOf(' ');
      arbologConfig.setDevicename(from_Serial.substring(pos+1));
      port.println( from_Serial.substring(pos+1) );
    }
    else if (from_Serial.startsWith("#setserver")){
      int pos = from_Serial.indexOf(' ');
      arbologConfig.setServerAdress(from_Serial.substring(pos+1));
      port.println( from_Serial.substring(pos+1) );
    }
    else if (from_Serial.startsWith("#setpin")){
      int pos = from_Serial.indexOf(' ');
      arbologConfig.setPin(from_Serial.substring(pos+1));
      port.println( from_Serial.substring(pos+1) );
    }
    else if (from_Serial.startsWith("#getconfig")){
      port.print("Pin: ");
      port.print(arbologConfig.getPin());      
      port.print(" Device: ");
      port.print(arbologConfig.getDevicename());      
      port.print(" Server: ");
      port.print(arbologConfig.getServerAdress());
      port.print(" GyroOffset: ");
      port.print(wsen.getGyroOffsetX());
      port.print(" ");
      port.print(wsen.getGyroOffsetY());
      port.print(" ");
      port.print(wsen.getGyroOffsetZ());
      port.print(" AccOffset: ");
      port.print(wsen.getAccOffsetX());
      port.print(" ");
      port.print(wsen.getAccOffsetY());
      port.print(" ");
      port.println(wsen.getAccOffsetZ());
    }
    else if (from_Serial.startsWith("#getversion")){
      port.print( "Version: ");
      port.print( VERSION);
      port.print( " Compiled: ");
      port.print( __DATE__ ); 
      port.print( " ");
      port.print(__TIME__ );
      port.print( " ");
      port.println(  __VERSION__ );
    }
    else if (from_Serial.length()){
      port.println(from_Serial);
      port.println("Unknown command");
    }
}

#endif