#include "USB/USBAPI.h"
#include "wsenisds.h"

byte WSENISDS::readRegister8(byte address){
    digitalWrite(CS, LOW);
    SPI.beginTransaction(SPISettings(SPI_CLOCK, SPI_BYTE, SPI_MODE));
    SPI.transfer(0x80 | address);
    byte data = SPI.transfer(0x00);
    SPI.endTransaction();
    digitalWrite(CS, HIGH);
    return data;
}

void WSENISDS::writeRegister8(byte address, byte value){
    digitalWrite(CS, LOW);
    SPI.beginTransaction(SPISettings(SPI_CLOCK, SPI_BYTE, SPI_MODE));
    SPI.transfer(address);
    SPI.transfer(value);
    SPI.endTransaction();
    digitalWrite(CS, HIGH);
}

int16_t WSENISDS::readRegister16(byte address){
    digitalWrite(CS, LOW);
    SPI.beginTransaction(SPISettings(SPI_CLOCK, SPI_BYTE, SPI_MODE));
    SPI.transfer(0x80 | address);
    int16_t data = SPI.transfer(0x00);
    int16_t hibyte = SPI.transfer(0x00);
    SPI.endTransaction();
    digitalWrite(CS, HIGH);
    return (data | (hibyte << 8));
}

void WSENISDS::setAccelerationRate( byte rate){
  byte data = readRegister8(CTRL1_XL);
  rate = rate << 4;
  data &= 0x0F;
  data = data | rate;
  writeRegister8(CTRL1_XL, data );
};

void WSENISDS::setAccelerationRange(int range){
    this->AccRange = range;
    byte state = readRegister8(CTRL1_XL);
    byte rangemask = 0; 
    AccFact = 0.061; 
    switch(range){
      case 4: rangemask = 2;
        AccFact = 0.122;
        break;
      case 8: rangemask = 3;
        AccFact = 0.244;  
        break;
      case 16:
        rangemask = 1;
        AccFact = 0.488;
        break;
    }
    rangemask = rangemask << 2;
    state = state & 0xF3;
    writeRegister8( CTRL1_XL, rangemask | state);
  }

int WSENISDS::getAccelerationRange(){
    if (this->AccRange)
      return this->AccRange;
    byte range = (readRegister8(CTRL1_XL) & 0x0C) >> 2; 
    switch (range){
      case 0: range = 2;
        AccFact = 0.061;
        break;
      case 1: range = 16;
        AccFact = 0.488;
        break;
      case 2: range = 4;
        AccFact = 0.122;
        break;
      case 3: range = 8;
        AccFact = 0.244;
        break;
    }
    this->AccRange = range;
    return range;
  }

byte WSENISDS::getAccelerationRate(){
    return readRegister8(CTRL1_XL) >> 4; 
  }

void WSENISDS::setGyroscopeRange(int range){ 
    this->GyroRange = range;
    byte state = readRegister8(CTRL2_G);
    // 250
    byte rangemask = 0;
    GyroFact = 8.75;
    switch(range){
      case 1000: 
        rangemask = 0b100;
        GyroFact = 35.0;
        break;
      case 2000: 
        rangemask = 0b110;
        GyroFact = 70.0;  
        break;
      case 500:
        rangemask = 0b10;
        GyroFact = 17.5;
        break;
      case 125:
        rangemask = 1;
        GyroFact = 4.375;
        break;
    }
    rangemask = rangemask << 1;
    state = state & 0xF1;
    writeRegister8( CTRL2_G, rangemask | state );
  }

int WSENISDS::getGyroscopeRange(){ 
    if (this->GyroRange)
      return this->GyroRange;
    byte range = (readRegister8(CTRL2_G) & 0x0B) >> 1;
    int retval = 0;  
    switch (range){
      case 0: retval = 250;
        break;
      case 1: retval = 125;
        break;
      case 2: retval = 500;
        break;
      case 4: retval = 1000;
        break;
      case 6: retval = 2000;
        break;
    }
    this->GyroRange = retval;
    return retval;
  }

void WSENISDS::setGyroscopeRate( byte rate){
  byte data = readRegister8(CTRL2_G);
  rate = rate << 4;
  data &= 0x0F;
  data = data | rate;
  writeRegister8(CTRL2_G, data );
};

byte WSENISDS::getGyroscopeRate(){
  return readRegister8(CTRL2_G) >> 4;
};
