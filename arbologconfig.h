#include "USB/USBAPI.h"
#include "api/Common.h"
#ifndef ARBLOGCONFIG_H
#define ARBLOGCONFIG_H

#include "arbolog.h"
#include "wsenisds.h"

#include <ArduinoJson.h>
#include "sdcard.h"

extern StaticJsonDocument<1024> JSONConfig;
extern sdcard SDCard;

const char *GYRO_ADDR = "%GY%";
const char *ACC_ADDR = "%AC%"; 
const char *DEVICE_ADDR = "%DV%"; 
const char *PIN_ADDR = "%PN%"; 
const char *SERVER_ADDR = "%SV%"; 
const char *END_ADDR = "%FN%"; 

extern WSENISDS wsen;

class ArbologConfig{
  private:
    String pin;
    int timeout; 
    String devicename;
    float a;
    float b;
    String serverAdress;
    int port;
    bool configmode;
    float longitude;
    float latitude;
    float height;
    int nVibration;
    int EEPROMSIZE;
    char buffer[1024];
    int gyroOffset[3];
    int accOffset[3];
    bool started;
  public: ArbologConfig(){
    started = false;
    nVibration = 0;
  }

  public: void startSampling(){
    started = true;
  }
  public: void stopSampling(){
    started = false;
  }

  public: bool isSampling(){
    return started;
  }

  private: bool isEEPROMinitialized(){
    if (EEPROM.isValid()
        && EEPROM.read(0) == '%'  
        && EEPROM.read(1) == 'G'
        && EEPROM.read(2) == 'Y'
        && EEPROM.read(3) == '%')
        return true;
    return false;
  }

  private: void getGyroOffsetsFromEEPROM(int *o){
    SerialUSB.println("Config: Get Gyroscope Offset.");
    if (!EEPROM.isValid()) {
      o[0] = 0;
      o[1] = 0;
      o[2] = 0;
      wsen.setGyroOffsetX(o[0]);
      wsen.setGyroOffsetY(o[1]);
      wsen.setGyroOffsetZ(o[2]);
      return;
    }
    int addr = 0;
    char point[28];
//    int8_t* point = (int8_t *) o;
    while(addr < EEPROMSIZE - 5){
      if (EEPROM.read(addr++) == '%' 
        && EEPROM.read(addr++) == 'G'
        && EEPROM.read(addr++) == 'Y'
        && EEPROM.read(addr++) == '%')
      break;
    }
    if (addr == EEPROMSIZE)
      return;
    int index = 0;
    char content;
    do{
        content = EEPROM.read(addr++);
        point[index++] = content;
     }
     while (content != '%');
     point[index-1] = 0;

    SerialUSB.println("Config: Got Gyroscope Offset.");
    
    sscanf(point, "%d,%d,%d", &o[0], &o[1], &o[2]);
    SerialUSB.println(point);
    SerialUSB.print("Read Gyro:");
    SerialUSB.print(o[0]);
    SerialUSB.print(" ");
    SerialUSB.print(o[1]);
    SerialUSB.print(" ");
    SerialUSB.println(o[2]);
    
    wsen.setGyroOffsetX(o[0]);
    wsen.setGyroOffsetY(o[1]);
    wsen.setGyroOffsetZ(o[2]);
  }

  private: void getAccOffsetsFromEEPROM(int *o){
    SerialUSB.println("Config: Get Acc Offset.");
    if (!EEPROM.isValid()) {
      o[0] = 0;
      o[1] = 0;
      o[2] = 0;
      return;
    }
    int addr = 0;
    char point[28];
    while(addr < EEPROMSIZE - 5){
      if (EEPROM.read(addr++) == '%' 
        && EEPROM.read(addr++) == 'A'
        && EEPROM.read(addr++) == 'C'
        && EEPROM.read(addr++) == '%')
      break;
    }
    if (addr >= EEPROMSIZE)
      return;
    int index = 0;
    char content;
    do{
        content = EEPROM.read(addr++);
        point[index++] = content;
     }
     while (content != '%');
     point[index-1] = 0;
    SerialUSB.println("Config: Got Acc Offset.");
    
    sscanf(point, "%d,%d,%d", &o[0], &o[1], &o[2]);
    SerialUSB.println(point);
    SerialUSB.print("Read Acc:");
    SerialUSB.print(o[0]);
    SerialUSB.print(" ");
    SerialUSB.print(o[1]);
    SerialUSB.print(" ");
    SerialUSB.println(o[2]);

    accOffset[0] = o[0];
    accOffset[1] = o[1];
    accOffset[2] = o[2];
  }

  public: void incVibration(){
    nVibration++;
  }

  public: int getVibration(){
    return (nVibration);
  }

  public: String getServerAdress(){
    return (serverAdress);
  }

  public: void setServerAdress(String serverAdress){
    this->serverAdress = serverAdress;
  }

  public: String getDevicename(){
    return (devicename);
  }

  public: void setDevicename(String deviceName){
    devicename = deviceName;
  }

  public: String getPin(){
    return (pin);
  }

  public: void setPin(String pin){
    this->pin = pin;
  }

  public: float getAkkuVoltage(){
    int sensorValue = analogRead(VOLTAGEPIN);
    return (2 * sensorValue * (V_REFERENCE / ANALOG_RESOLUTION));
  }

  public: float getTemperature(){
    int sensorValue = analogRead(TEMPERATUREPIN);
    float Vout = sensorValue * (V_REFERENCE / ANALOG_RESOLUTION);
    // Widerstand des NTC berechnen
    float Rntc = (Vout * 3000.0) / (V_REFERENCE - Vout);
    // Temperatur in Kelvin berechnen
    float tempK = 3988.0 / (log(Rntc / 3000.0) + 3988.0 / 298.15);
    // Temperatur in Celsius umrechnen
    float tempC = tempK - 273.15;
    return (tempC);
  }

/*
  public: void writeGyroOffset(void){
    int16_t o[3];
    o[0] = wsen.getGyroOffsetX();
    o[1] = wsen.getGyroOffsetY();
    o[2] = wsen.getGyroOffsetZ();
    int8_t* point = (int8_t *) 0;
    for (int GYRO_ADDR=0; i< 6; i++)
      EEPROM.write( i, point[i]);
    EEPROM.commit();
  }
*/
  public: void begin(){
    pinMode(BUTTON_PIN, INPUT);
    pinMode(VOLTAGEPIN, INPUT);
    pinMode(TEMPERATUREPIN, INPUT);
    analogReadResolution(10);
#ifdef SDCARD
    // read config from SD card
    SDCard.readConfig();
    devicename = (char *) JSONConfig["device"].as<const char*>();
    Serial.println(devicename);
    serverAdress = (char *) JSONConfig["server"].as<const char*>();
    Serial.println(serverAdress);
    port = (int) JSONConfig["port"].as<int>();
    Serial.print("port: ");
    Serial.println(port);
    longitude = JSONConfig["longitude"].as<float>();
    Serial.print("longitude: ");
    Serial.println(longitude);
    latitude = JSONConfig["latitude"].as<float>();
    Serial.print("latitude: ");
    Serial.println(latitude);
    height = JSONConfig["height"].as<float>();
    Serial.print("height: ");
    Serial.println(height);
    if (JSONConfig["pin"]){
      Serial.print("pin: ");
      setPin(JSONConfig["pin"].as<const char*>());
    }
    Serial.println(pin);
#else
    EEPROMSIZE = EEPROM.length();
    if (!isEEPROMinitialized()) 
      return;
    getGyroOffsetsFromEEPROM(gyroOffset);
    getAccOffsetsFromEEPROM(accOffset);
    pin = getPinFromEEPROM(); //"1862";
    devicename = getDeviceFromEEPROM();
    serverAdress = getServernameFromEEPROM(); //"http://gdg5tt9hs96lodss.myfritz.net:8088";
  #endif
  configureWifiAdapter();
  }

  public: void configureWifiAdapter(void){
    char *ssid;
    char *pass;
    Serial1.println("");
    String toSend("setssids ");
    int index = 0;
    while (ssid = (char *) JSONConfig["WLAN"][index]["SSID"].as<const char*>()){
      if (index)
        toSend = toSend + ",";
      toSend = toSend + ssid;
      toSend = toSend + " ";
      pass = (char *) JSONConfig["WLAN"][index]["passcode"].as<const char*>();
      if (pass) toSend = toSend + pass;
      index++;
      }
    Serial1.println(toSend);
    toSend = "sethostname ";
    toSend = toSend + (char *) JSONConfig["device"].as<const char*>();;
    Serial1.println(toSend);
    toSend = "setclientversion ";
    toSend = toSend + VERSION;
    Serial1.println(toSend);
    toSend = "setserver ";
    toSend = toSend + serverAdress;
    Serial1.println(toSend);
    Serial1.println("start");
  }

// The first entry up to \n
  private: String getPinFromEEPROM(void){
    if (!EEPROM.isValid())
      return "";
    int addr = 0;
    while(addr < EEPROMSIZE - 5){
      if (EEPROM.read(addr++) == '%' 
        && EEPROM.read(addr++) == 'P'
        && EEPROM.read(addr++) == 'N'
        && EEPROM.read(addr++) == '%')
      break;
    }
    if(addr >= EEPROMSIZE)
      return ("");
    int index = 0;
    char content;
    do{
        content = EEPROM.read(addr++);
        buffer[index++] = content;
    }
    while (content != '%');
    buffer[index-1] = 0;
    return buffer;
  }
  
  public: String getPasscode(const char* SSID){
    char* ssid;
    char* pass;
    int index = 0;
    while (ssid = (char *) JSONConfig["WLAN"][index]["SSID"].as<const char*>()){
      if (!strcmp(SSID,ssid)){
        pass = (char *) JSONConfig["WLAN"][index]["passcode"].as<const char*>();
        return (pass);
      }
      index++;
    }
    return ("");
  }
  
  private: String getDeviceFromEEPROM(void){
    if (!EEPROM.isValid())
      return "";
    int addr = 0;
    while(addr < EEPROMSIZE - 5){
      if (EEPROM.read(addr++) == '%' 
        && EEPROM.read(addr++) == 'D'
        && EEPROM.read(addr++) == 'V'
        && EEPROM.read(addr++) == '%')
      break;
    }
    if(addr >= EEPROMSIZE)
      return ("");
    int index = 0;
    char content;
    do{
        content = EEPROM.read(addr++);
        buffer[index++] = content;
     }
     while (content != '%');
    buffer[index-1] = 0;
    return buffer;
  }

  private: String getServernameFromEEPROM(void){
    if (!EEPROM.isValid())
      return "";
    int addr = 0;
    while(addr < EEPROMSIZE - 5){
      if (EEPROM.read(addr++) == '%' 
        && EEPROM.read(addr++) == 'S'
        && EEPROM.read(addr++) == 'V'
        && EEPROM.read(addr++) == '%')
      break;
    }
    if(addr >= EEPROMSIZE)
      return ("");
    int index = 0;
    char content;
    do{
        content = EEPROM.read(addr++);
        buffer[index++] = content;
     }
     while (content != '%');
    buffer[index-1] = 0;
    return buffer;
  }

  public: void updateEEPROM(void){
    int addr = 0;
    int16_t o[3];
    char buffer[20];
    int strlength = 0;
  // Gyroscope Offset
    for (int i =0; i < 4; i++)
      EEPROM.write( addr++, GYRO_ADDR[i]);
    sprintf(buffer, "%d,%d,%d", wsen.getGyroOffsetX(), wsen.getGyroOffsetY(), wsen.getGyroOffsetZ());
    SerialUSB.print("Length Gyro:");
    SerialUSB.println(strlen(buffer));
    for (int i=0; i< strlen(buffer); i++)
      EEPROM.write( addr++, buffer[i]);
  // ACC data
    for (int i =0; i < 4; i++)
      EEPROM.write( addr++, ACC_ADDR[i]);
    sprintf(buffer, "%d,%d,%d", wsen.getAccOffsetX(), wsen.getAccOffsetY(), wsen.getAccOffsetZ());
    SerialUSB.print("Length Acc:");
    SerialUSB.println(strlen(buffer));
    for (int i=0; i< strlen(buffer); i++)
      EEPROM.write( addr++, buffer[i]);
    // Devicename
    for (int i =0; i < 4; i++)
      EEPROM.write( addr++, DEVICE_ADDR[i]);
    const char *devaddrs = devicename.c_str();
    strlength = devicename.length();
    if (!strlength || strlength > 20)
      for (int i=0; i< 6; i++)
        EEPROM.write( addr++, 'D');
    else
      for (int i=0; i< strlength; i++)
        EEPROM.write( addr++, devaddrs[i]);
    // PIN
    for (int i =0; i < 4; i++)
      EEPROM.write( addr++, PIN_ADDR[i]);
    const char *pinaddrs = pin.c_str();
    strlength = pin.length();
    if (!strlength || strlength > 20)
      for (int i=0; i< 6; i++)
        EEPROM.write( addr++, 'P');
    else
      for (int i=0; i< strlength; i++)
        EEPROM.write( addr++, pinaddrs[i]);
    // Server adress
    for (int i =0; i < 4; i++)
      EEPROM.write( addr++, SERVER_ADDR[i]);
    const char *srvaddrs = serverAdress.c_str();
    strlength = serverAdress.length();
    if (!strlength || strlength > 50)
      for (int i=0; i< 6; i++)
        EEPROM.write( addr++, 'D');
    else
      for (int i=0; i< strlength; i++)
        EEPROM.write( addr++, srvaddrs[i]);
    for (int i =0; i < 4; i++)
      EEPROM.write( addr++, END_ADDR[i]);
    
    EEPROM.commit();
  }

  public: void dumpEEPROM(){
    for(int i=0; i < 256; i++){
      SerialUSB.print((char) EEPROM.read(i));
      if (!(i % 80) ) SerialUSB.println();
    }
  }




};

#endif