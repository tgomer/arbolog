#include "api/Common.h"
#ifndef WSENISDS_H
#define WSENISDS_H

/* 
  Arduino driver for Würth WSEN Gyropscope Accelerator sensor
  https://www.we-online.com/components/products/manual/2536030320001_Manual_UM_WSEN-ISDS_2536030320001_rev1.2.pdf
*/

#include <Arduino.h>
#include <SPI.h>

// SPI control

#define SPI_CLOCK 1000000
#define SPI_MODE SPI_MODE0
#define SPI_BYTE MSBFIRST

// 0x00-05 Reserved1 - - Reserved
#define FIFO_CTRL1 0x06     // FIFO configuration register
#define FIFO_CTRL2 0x07             
#define FIFO_CTRL3 0x08             
#define FIFO_CTRL4 0x09
#define FIFO_CTRL5 0x0A 
#define DRDY_PULSE_CFG 0x0B
// 0x0C Reserved1 - - Reserved
#define INT0_CTRL 0x0D       // INT0 pin control
#define INT1_CTRL 0x0E       // INT1 pin control
#define DEVICE_ID 0x0F       // Device address
#define CTRL1_XL 0x10        // Acceleration and gyroscope register
#define CTRL2_G 0x11
#define CTRL3_C 0x12 
#define CTRL4_C 0x13 
#define CTRL5_C 0x14 
#define CTRL6_C 0x15 
#define CTRL7_G 0x16 
#define CTRL8_XL 0x17 
#define CTRL9_XL 0x18 
#define CTRL10_C 0x19 
#define MASTER_CONFIG 0x1A    // I2C master configuration register
#define WAKE_UP_SRC 0x1B      // Interrupt register
#define TAP_SRC 0x1C  
#define D6D_SRC 0x1D  
#define STATUS_REG 0x1E       // Status data register for GP and OIS data
// 0x1F Reserved1 - - Reserved
#define T_OUT_L 0x20 // Temperature data registers
#define T_OUT_H 0x21 
#define G_X_OUT_L 0x22 // Gyroscope data registers
#define G_X_OUT_H 0x23 
#define G_Y_OUT_L 0x24 
#define G_Y_OUT_H 0x25 
#define G_Z_OUT_L 0x26 
#define G_Z_OUT_H 0x27 
#define XL_X_OUT_L 0x28 // Acceleration data registers
#define XL_X_OUT_H 0x29 
#define XL_Y_OUT_L 0x2A 
#define XL_Y_OUT_H 0x2B 
#define XL_Z_OUT_L 0x2C 
#define XL_Z_OUT_H 0x2D  
// 0x2E-39 Reserved1 - - Reserved
#define FIFO_STATUS1  0x3A    // FIFO status registers
#define FIFO_STATUS2 0x3B  
#define FIFO_STATUS3 0x3C  
#define FIFO_STATUS4 0x3D 
#define FIFO_DATA_OUT_L 0x3E  //FIFO data registers
#define FIFO_DATA_OUT_H 0x3F 
// 0x40-52 Reserved1 - - Reserved
#define FUNC_SRC1 0x53 // Interrupt registers
// 0x54-57 Reserved1 - - Reserved
#define TAP_CFG 0x58 // Interrupt registers
#define TAP_THS_6D 0x59 
#define INT_DUR2 0x5A 
#define WAKE_UP_THS 0x5B 
#define WAKE_UP_DUR 0x5C 
#define FREE_FALL 0x5D 
#define MD1_CFG 0x5E 
#define MD2_CFG 0x5F 
// 0x60-72 Reserved1 - - Reserved
#define X_OFS_USR 0x73 // Acceleration user offset correction
#define Y_OFS_USR 0x74  
#define Z_OFS_USR 0x75 
// 0x76-7F Reserved

// sample rates

#define POWERDOWN 0b0000
#define RATE12_5Hz    0b0001 
#define RATE26Hz      0b0010
#define RATE52Hz      0b0011
#define RATE104Hz     0b0100
#define RATE208Hz     0b0101
#define RATE416Hz     0b0110
#define RATE833Hz     0b0111
#define RATE1_66kHz   0b1000
#define RATE3_33kHz   0b1001
#define RATE6_66kHz   0b1010

/*
±2g 0.061 mg /digit
±4g 0.122 mg /digit
±8g 0.244 mg /digit
±16g 0.488 mg /digit

±125dps 4.375 mdps /digit
±250dps 8.75 mdps /digit
±500dps 17.5 mdps /digit
±1000dps 35 mdps /digit
±2000dps 70
*/ 

class WSENISDS {
  private: int AccRange;
           int16_t AccOffsetX = 0;
           int16_t AccOffsetY = 0;
           int16_t AccOffsetZ = 0;
           int GyroRange;
           int16_t GyroOffsetX = 0;
           int16_t GyroOffsetY = 0;
           int16_t GyroOffsetZ = 0;
           float AccFact;
           float GyroFact; 
  private: int CS;     // Chip select
  public: void setCSPin(int pin){
    CS = pin;
    pinMode(CS, OUTPUT);
  }
  public: byte readRegister8(byte address);
  public: void writeRegister8(byte address, byte value);
  public: int16_t readRegister16(byte address);
  public: byte getDeviceID(void){ return (readRegister8(DEVICE_ID)); };
  public: int16_t getTemperature(){return readRegister16(T_OUT_L);};
  public: float getDegreeCentigrade(){return (readRegister16(T_OUT_L)/256.0 + 25.0);};

  public: int16_t readAccXRaw(){ return readRegister16(XL_X_OUT_L);};
  public: int16_t readAccYRaw(){ return readRegister16(XL_Y_OUT_L);}
  public: int16_t readAccZRaw(){ return readRegister16(XL_Z_OUT_L);};

  public: float readAccX(){ return AccFact * readRegister16(XL_X_OUT_L);};
  public: float readAccY(){ return AccFact * readRegister16(XL_Y_OUT_L);};
  public: float readAccZ(){ return AccFact * readRegister16(XL_Z_OUT_L);};

  public: void calibrateGyroscope(){
    long X=0,Y=0,Z=0;
    for (int i = 0; i < 2000; i++){
      X += readGyroXRaw(); 
      Y += readGyroYRaw(); 
      Z += readGyroZRaw();
      delay(3); 
    }
    GyroOffsetX = (int16_t) X/2000; 
    GyroOffsetY = (int16_t) Y/2000; 
    GyroOffsetZ = (int16_t) Z/2000; 
  };

  public: void calibrateAcc(){
    long X=0,Y=0,Z=0;
    for (int i = 0; i < 2000; i++){
      X += readAccXRaw(); 
      Y += readAccYRaw(); 
      Z += readAccZRaw();
      delay(3); 
    }
    AccOffsetX = (int16_t) X/2000; 
    AccOffsetY = (int16_t) Y/2000; 
    AccOffsetZ = (int16_t) Z/2000; 
  };


  public: void setGyroOffsetX(int16_t offset){ GyroOffsetX = offset;};
  public: void setGyroOffsetY(int16_t offset){ GyroOffsetY = offset;};
  public: void setGyroOffsetZ(int16_t offset){ GyroOffsetZ = offset;};

  public: int16_t getGyroOffsetX(){ return GyroOffsetX;};
  public: int16_t getGyroOffsetY(){ return GyroOffsetY;};
  public: int16_t getGyroOffsetZ(){ return GyroOffsetZ;};


  public: void setAccOffsetX(int16_t offset){ AccOffsetX = offset;};
  public: void setAccOffsetY(int16_t offset){ AccOffsetY = offset;};
  public: void setAccOffsetZ(int16_t offset){ AccOffsetZ = offset;};

  public: int16_t getAccOffsetX(){ return AccOffsetX;};
  public: int16_t getAccOffsetY(){ return AccOffsetY;};
  public: int16_t getAccOffsetZ(){ return AccOffsetZ;};

/*
±2g 0.061 mg /digit
±4g 0.122 mg /digit
±8g 0.244 mg /digit
±16g 0.488 mg /digit

±125dps 4.375 mdps /digit
±250dps 8.75 mdps /digit
±500dps 17.5 mdps /digit
±1000dps 35 mdps /digit
±2000dps 70
*/ 


  public: int16_t readGyroXRaw(){return readRegister16(G_X_OUT_L);};
  public: int16_t readGyroYRaw(){return readRegister16(G_Y_OUT_L);};
  public: int16_t readGyroZRaw(){return readRegister16(G_Z_OUT_L);};    

  public: float readGyroX(){return GyroFact * (readRegister16(G_X_OUT_L) - GyroOffsetX);};
  public: float readGyroY(){return GyroFact * (readRegister16(G_Y_OUT_L) - GyroOffsetY);};
  public: float readGyroZ(){return GyroFact * (readRegister16(G_Z_OUT_L) - GyroOffsetZ);};    

  public: void setGyroscopeRate( byte rate);
  public: byte getGyroscopeRate();
  public: void setAccelerationRate( byte rate);
  public: byte getAccelerationRate();
  
  public: void setAccelerationRange(int range);
  public: int getAccelerationRange();

  public: void setGyroscopeRange(int range);
  public: int getGyroscopeRange();
  
  public: WSENISDS(){
    AccRange = 0;
    GyroRange = 0;
    AccFact = 0.0;
    GyroFact = 0.0; 
  }
  public: void begin(){ 
    SPI.begin(); 
    writeRegister8(CTRL1_XL, 0);
    getAccelerationRange();
    getGyroscopeRange();
  };
};

#endif