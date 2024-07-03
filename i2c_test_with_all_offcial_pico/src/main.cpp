/*
  this project adapt the i2c_test_with_all_test_1 to offical rp2040 pico

  the changes is
  1. r0~3 is 16 17 18 19
  2. c0~3 is 22 26 27 28
  
  led is still 0~15
  i2c is still sda 20 scl 21

  this is the full function code for a cell unit  
*/
#include <Arduino.h>

#include <Wire.h>
#include <elapsedMillis.h>
#include "LedMatrix.h"
#include "KeyMatrix.h"

#define ADDRESS 0x01

KeyMatrix Km1;
LedMatrix Mx1;
elapsedMillis m;

void sendKey();
void getLed(int a);
void setup() {
  // put your setup code here, to run once:
  // Mx1.testLed();

  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin(ADDRESS);
  Wire.onRequest(sendKey);
  Wire.onReceive(getLed);
  m = 0;
}

void loop() {
  
  if (m > 16)
  {
    Km1.vUpdateMatrix();
    Mx1.updateLeds();
    m = 0;
  }
}

void sendKey()

{
  // return how many bytes?
  for (uint8_t i = 0; i < Km1.p_keyNum; i++)
  {
    Wire.write(Km1.iGetKey(i));
  }
}

void getLed(int a)
{
  if(Wire.available())
  {
    for (int i=0; i<16; i++)
    {
      uint8_t level = Wire.read();
      Mx1.setLedBrightness(i, level);
    }
  }
  while(Wire.available())
  {
      Wire.read();
  }
}
