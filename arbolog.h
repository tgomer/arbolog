#ifndef ARBOLOG_H
#define ARBOLOG_H

/*
Arbolog control programm 

Arduino SAMD Board 1.8.14

Analog Pins:
A0 -> PA02   -  leer
A1 -> PB02   -  Vibrationssensor (digital)
A2 -> PB03   -  leer
A3 -> PA04   -  Temperature
A4 -> PA05   -  Akku
A5 -> PA06   -  leer
A6 -> PA07   -  leer
AREF -> PA03 -  Reference
Digital Pins:
0  -> PA22   -  SRAM Chip Select 
26 -> PA12   -  Adrastea Shutdown/Wakeup
2  -> PA10   -  WSENS Chip select
28 -> PA14   -  Button
27 -> PA13   -  LEDs

SD select -  PIN_SPI1_SS;

EEPROM length: 1024
*/

#define VERSION "1.00"

#define TEMPERATUREPIN A3
#define VOLTAGEPIN A4

#define SRAM_SELECT_PIN		0

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 2 

#define VIBRATION_PIN A1
#define V_REFERENCE 3.3   // 2.5
#define ANALOG_RESOLUTION 1023.0 // 10 bit // 4095.0 // 12 bit

#define BUTTON_PIN 28
#define NEOPIXEL_PIN 27

#define WLAN
#define DEBUG true
#define MODE_1A
 
#define SDCARD

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

#define MODULE_POWER 31
#define MODULE_WAKEUP 20

#define ADASTEASD_PIN 26

#define CSWSENS_PIN 2 // WSENS

#include <Arduino.h>

float getTemperature8016( float resistance);

#endif