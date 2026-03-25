#ifndef __SDCARD_H__
#define __SDCARD_H__

#include <SD.h>
#include <ArduinoJson.h>

extern StaticJsonDocument<1024> JSONConfig;

class sdcard{
  private: const int chipSelect = PIN_SPI1_SS;

  public: void begin(void){
  }

  public: bool readConfig(const char* configFile = "arbolog.cfg"){
    Serial.println("readConfig");
    begin();
    Serial.println(configFile);
    SD.begin(chipSelect);
    File dataFile = SD.open(configFile);
    if (!dataFile)
      Serial.println(F("Failed to open file"));
    DeserializationError error = deserializeJson(JSONConfig, dataFile);
    if (error)
      Serial.println(F("Failed to read file, using default configuration"));
    dataFile.close();
    return true;
  }
};

#endif
