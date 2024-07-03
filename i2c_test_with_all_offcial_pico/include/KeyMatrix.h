/*
  1. r0~3 is 16 17 18 19
  2. c0~3 is 22 26 27 28
*/

#ifndef KEYMATRIX_H
#define KEYMATRIX_H

#include <Arduino.h>

class KeyMatrix
{
  private:
    const uint8_t rowPin[4] = {16, 17, 18, 19};
    const uint8_t colPin[4] = {22, 26, 27, 28};
    uint8_t p_keyAction[16];
  public:
    KeyMatrix();
    void vPrintKeyAction();
    void vUpdateMatrix();
    uint8_t iGetKey(uint8_t index) const;

    const uint8_t p_keyNum = 16;
};

#endif