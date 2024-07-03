#include "KeyMatrix.h"


KeyMatrix::KeyMatrix()
{
  for (uint8_t r : rowPin)
  {
    pinMode(r, INPUT_PULLUP);
  }

  for (uint8_t c : colPin)
  {
    pinMode(c, INPUT_PULLUP);
  }

  for (uint8_t i = 0; i<p_keyNum; i++)
  {
    p_keyAction[i] = 0; 
  }
  // Serial.begin(115200);
  // Serial.println("keypad is initialized.");
}

void KeyMatrix::vPrintKeyAction()
{ 
  Serial.println("Print Begin");
  Serial.println("----------------------------------");
  for (uint8_t i; i<p_keyNum; i++)          
  {
    if (p_keyAction[i] == 0) Serial.print('O');
    else if (p_keyAction[i] == 1) Serial.print('X');
    else Serial.print('F');

    Serial.print(" ");  
    if ( (i+1)%4 == 0 ) Serial.println();
  }
  Serial.println("----------------------------------");
  Serial.println("Print End");
}

void KeyMatrix::vUpdateMatrix()
{
  for (uint8_t c = 0; c<4; c++)
  { 
    pinMode(colPin[c], OUTPUT);
    digitalWrite(colPin[c], LOW);
    for (uint8_t r = 0; r<4; r++)
    {
      uint8_t index = r*4+c;
      p_keyAction[index] = !(digitalRead(rowPin[r]));
    }
    pinMode(colPin[c], INPUT_PULLUP);
  }
}

uint8_t KeyMatrix::iGetKey(uint8_t index) const
{
  return p_keyAction[index];
}