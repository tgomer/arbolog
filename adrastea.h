#ifndef __ADRASTEA_H__
#define __ADRASTEA_H__

// Docs:
// User manual (commander)
// https://www.we-online.com/components/products/manual/2615011136000_Manual-um-adrastea-i-2615011136000-v1-0_rev1.1.pdf
// AT commands:
// https://www.we-online.com/components/media/o691492v8984%20Manual-um-acm-adrastea-i-2615011136000-V1-0%20%28rev1%29.pdf


#define ADRASTEA_DEFAULT_BAUDRATE 115200
#define ADRASTEA_DEFAULT_SHUTDOWNPIN 26
// #define DEBUG
#include <Regexp.h>

#define READDATATIMEOUT 300

void matchPosition(const char *match,          // matching string (not null-terminated)
                   const unsigned int length,  // length of matching string
                   const MatchState &ms);      // MatchState in use (to get captures)

void matchSMSHeader(const char *match,          // matching string (not null-terminated)
                    const unsigned int length,  // length of matching string
                    const MatchState &ms);      // MatchState in use (to get captures)

void matchSerial(const char *match,          // matching string (not null-terminated)
                 const unsigned int length,  // length of matching string
                 const MatchState &ms);      // MatchState in use (to get captures)

typedef struct {
  bool fromSatellite;
  int noSats;
  char time[9];
  char date[11];
  char latitude[13];
  char longitude[13];
  char height[13];
  //  time_t UTC;
  char accuracy[5];
  char velocity[13];
} Adrastea_Satellite_t;

typedef struct {
  int index;
  char status[11];
  char number[24];
  char date[9];
  char time[9];
} Adrastea_SMSHeader_t;

static void *AdrasteaInstance;

class Adrastea {
private:
  Uart *_uart;
  int _baudRate;
  int _shutDownPin;
  Adrastea_SMSHeader_t SMSHeaders[64];
public:
  Adrastea_Satellite_t _satellite;
  Adrastea_SMSHeader_t SMSHeader;
  char _serial[16];
  const char *regexFix =
    "%%IGNSSINFO:%s(%d),\"([%d:]+)\",\"([%d/]+)\",\"([%d%.]+)\",\"([%d%.]+)\",\"([%d%.]+)\",%d+,([%d%.]+),\"([%d%.]+).*";
  const char *regexSMS =
    "%+CMGL:%s(%d+),\"(REC %a+)\",\"([%d%+]+)\",,\"([%d/]+),([%d:]+)%+%d+\".*";
  const char *regexSerial =
    ".*(AT%+GSN)%s*(%d+).*";

public:
  Adrastea() {
    memset(&_satellite, 0, sizeof(_satellite));
    memset(&_serial, 0, sizeof(_serial));
    AdrasteaInstance = this;
  }

public:
  void setUart(Uart *uart) {
    _uart = uart;
    _uart->begin(ADRASTEA_DEFAULT_BAUDRATE);
    sendData("map", 300);
  }

public:
  void readSatelliteData() {
    int index;
    String _fromAdrastea;
    sendData("AT+CFUN=0", READDATATIMEOUT);
    sendData("AT%IGNSSACT=1,1", READDATATIMEOUT);
    sendData("AT%IGNSSCFG=\"SET\",\"SAT\",\"GPS\",\"GLONASS\"", READDATATIMEOUT);
    int trial = 0;
    while (_satellite.noSats == 0 && trial < 10) {
      delay(1000);
      _fromAdrastea = sendData("AT%IGNSSINFO=\"SAT\"", READDATATIMEOUT);
      if (_fromAdrastea.indexOf("OK") != -1) {
        index = _fromAdrastea.indexOf("%IGNSSINFO:");
        index += 12;
        sscanf(_fromAdrastea.substring(index).c_str(), "%i", &_satellite.noSats);
        Serial.print("Sats:");
        Serial.println(_satellite.noSats);
      }
      trial++;
    }
    if (!_satellite.noSats)
      return;
    trial = 0;
    while (trial < 3) {
      _fromAdrastea = sendData("AT%IGNSSINFO=\"FIX\"", READDATATIMEOUT);
      trial++;
      if (_fromAdrastea.indexOf("%IGNSSINFO: 0") == -1)
        break;
    }
    MatchState ms((char *)_fromAdrastea.c_str());
    unsigned long count = ms.GlobalMatch(regexFix, matchPosition);
    if (!count) {
      _satellite.fromSatellite = false;
      // read from properties
    }
    /*Serial.print("Time: ");
    Serial.println( _satellite.time);
    Serial.print("Date: ");
    Serial.println( _satellite.date);
    Serial.print("Latitude: "); 
    Serial.println( _satellite.latitude);
    Serial.print("Longitude: "); 
    Serial.println( _satellite.longitude);
    Serial.print("Height: "); 
    Serial.println( _satellite.height);
    Serial.print("Accuracy: "); 
    Serial.println( _satellite.accuracy);
    Serial.print("Velocity: "); 
    Serial.println( _satellite.velocity);*/
  }

public:
  int getSats() {
    return _satellite.noSats;
  }
public:
  String getTime() {
    return _satellite.time;
  }
public:
  String getDate() {
    return _satellite.date;
  }
public:
  void setSDPin(int SDPin) {
    _shutDownPin = SDPin;
    pinMode(_shutDownPin, OUTPUT);
  }
public:
  void wakeup() {
    digitalWrite(_shutDownPin, LOW);
  }
public:
  void shutdown() {
    digitalWrite(_shutDownPin, HIGH);
  }
public:
  String readSMS(int index) {
    sendData("AT+CMGF=1", READDATATIMEOUT);  // Text mode
    String command = "AT+CMGR=" + String(index, DEC);
    String _fromAdrastea = sendData(command, READDATATIMEOUT);
    char *messageStart = strchr(_fromAdrastea.c_str(), '\n');
    messageStart = strstr(messageStart, "+CMGR:");
    messageStart = strchr(messageStart, '\n');
    if (!messageStart)
      return ("ERROR");
    messageStart++;
    String retval = messageStart;
    Serial.println(messageStart);
    int messageEnd = retval.lastIndexOf("\nOK");
    if (messageEnd == -1)
      return ("ERROR");
    return retval.substring(0, messageEnd - 1);
  }

public:
  Adrastea_SMSHeader_t *getSMSHeaders() {
    memset(SMSHeaders, 0, sizeof(SMSHeaders));
    SerialUSB.println(sizeof(SMSHeaders));
    sendData("AT+CMGF=1", READDATATIMEOUT);  // Text mode
    String _fromAdrastea = sendData("AT+CMGL", READDATATIMEOUT);
    char *messageHeader = strstr(_fromAdrastea.c_str(), "+CMGL:");
    int index = 0;
    while (messageHeader) {
      MatchState msSMS(messageHeader);
      unsigned long count = msSMS.GlobalMatch(regexSMS, matchSMSHeader);
      if (count == 1) {
        memcpy(&(SMSHeaders[index]), &SMSHeader, sizeof(SMSHeader));
        if (index == 63)
          break;
      }
      index++;
      messageHeader = strstr(messageHeader + 1, "+CMGL:");
    }
    SerialUSB.print("No of SMS:");
    SerialUSB.println(index);
    return SMSHeaders;
  }

public:
  void deleteSMS(int index) {
    sendData("AT+CFUN=1", READDATATIMEOUT);
    String command ="AT+CMGD=" + String(index, DEC);
    sendData(command, READDATATIMEOUT);
  }

public:
  bool sendSMS(String phoneNumber, String text) {
    sendData("AT+CFUN=1", READDATATIMEOUT);
    if (!sendData("AT+CMGS=\"" + phoneNumber + "\"", READDATATIMEOUT))
      return false;
    if (!sendData(text, READDATATIMEOUT))
      return false;
    _uart->write(26);  // End
    return true;
  }
public:
  bool setPIN(String PIN){
    String _fromAdrastea = sendData( "AT+CPIN=\"" + PIN + "\"", READDATATIMEOUT); 
    Serial.println ("AT+CPIN=\"" + PIN + "\"");
    Serial.println (_fromAdrastea);
    if ( _fromAdrastea != "OK")
      return false;
    return true;
  }

public:
  String getSerialNumber() {
    if (_serial[0] != 0)
      return _serial;
    String _fromAdrastea = sendData("AT+GSN", READDATATIMEOUT);
    MatchState msSerial((char *) _fromAdrastea.c_str());
//    unsigned long count = msSerial.GlobalMatch(regexSerial, matchSerial);
    return _serial;
  }

private:
  String sendData(String command, const int timeout) {
    String response = "";
    _uart->println(command);
    unsigned long int time = millis();
    while ((time + timeout) > millis()) {
      while (_uart->available()) {
        char c = _uart->read();
        response += c;
      }
    }
#ifdef DEBUG
    Serial.println(response);
#endif
    return response;
  }
};  // end of class declaration

// fill in SMSHeaderData
void matchSMSHeader(const char *match,          // matching string (not null-terminated)
                    const unsigned int length,  // length of matching string
                    const MatchState &ms)       // MatchState in use (to get captures)
{
  if (ms.level < 4) {
    Serial.println("Error in regex for SMS Headers");
    return;
  }
  char buffer[16];
  memset(&((Adrastea *)AdrasteaInstance)->SMSHeader, 0, sizeof(Adrastea_SMSHeader_t));
  ms.GetCapture(buffer, 0);
  sscanf(buffer, "%d", &((Adrastea *)AdrasteaInstance)->SMSHeader.index);
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->SMSHeader.status, 1);
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->SMSHeader.number, 2);
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->SMSHeader.date, 3);
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->SMSHeader.time, 4);
};

// fill in satelite data
void matchPosition(const char *match,          // matching string (not null-terminated)
                   const unsigned int length,  // length of matching string
                   const MatchState &ms)       // MatchState in use (to get captures)
{
  if (ms.level < 7) {
    Serial.println("Error in regex for fix");
    return;
  }
  memset(((Adrastea *)AdrasteaInstance)->_satellite.time, 0, sizeof(((Adrastea *)AdrasteaInstance)->_satellite.time));
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_satellite.time, 1);
  memset(((Adrastea *)AdrasteaInstance)->_satellite.date, 0, sizeof(((Adrastea *)AdrasteaInstance)->_satellite.date));
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_satellite.date, 2);
  memset(((Adrastea *)AdrasteaInstance)->_satellite.latitude, 0, sizeof(((Adrastea *)AdrasteaInstance)->_satellite.latitude));
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_satellite.latitude, 3);
  memset(((Adrastea *)AdrasteaInstance)->_satellite.longitude, 0, sizeof(((Adrastea *)AdrasteaInstance)->_satellite.longitude));
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_satellite.longitude, 4);
  memset(((Adrastea *)AdrasteaInstance)->_satellite.height, 0, sizeof(((Adrastea *)AdrasteaInstance)->_satellite.height));
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_satellite.height, 5);
  memset(((Adrastea *)AdrasteaInstance)->_satellite.accuracy, 0, sizeof(((Adrastea *)AdrasteaInstance)->_satellite.accuracy));
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_satellite.accuracy, 6);
  memset(((Adrastea *)AdrasteaInstance)->_satellite.velocity, 0, sizeof(((Adrastea *)AdrasteaInstance)->_satellite.velocity));
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_satellite.velocity, 7);
  ((Adrastea *)AdrasteaInstance)->_satellite.fromSatellite = true;
};

// fill in serial data
void matchSerial(const char *match,          // matching string (not null-terminated)
                 const unsigned int length,  // length of matching string
                 const MatchState &ms)       // MatchState in use (to get captures)
{
  if (ms.level < 2) {
    Serial.println("Error in regex for serial");
    return;
  }
  ms.GetCapture(((Adrastea *)AdrasteaInstance)->_serial, 1);
};

#endif