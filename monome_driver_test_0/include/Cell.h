#ifndef CELL_H
#define CELL_H

#include <Arduino.h>
#include <Wire.h>

#define KEY_EVENT_MAXNUM 16
#define CELL_KEY_RELEASE 0
#define CELL_KEY_PRESS 1
#define CELL_KEY_NOCHANGE 255

class Cell
{
private:
  const uint8_t p_iAddress;
  const uint8_t p_iSizeX;
  const uint8_t p_iSizeY;
  const uint8_t p_iElementNum;

  uint8_t *p_aKeyAction;
  uint8_t *p_aLedLevel;

  TwoWire *p_pI2C;

  bool p_bLedUpdateFlag = 0;

  // void vEnableLedUpdate();

public:
  Cell() = default; //  create a empty cell
  Cell(uint8_t address, TwoWire *i2c, uint8_t sizeX=4, uint8_t sizeY=4);
  ~Cell();


  // key
  uint8_t p_iKeyEvent[KEY_EVENT_MAXNUM][3]; // {event type, x, y}
  uint8_t p_iKeyEventFlag = 0;  // 0 no update; 1 update
  
  void resetKeyEvent();
  void vGetKey(); // get all key actions updated
  

  // led
  void vSetLed(uint8_t x, uint8_t y, uint8_t level); // set Singel Led
  void vSetLed(uint8_t index, uint8_t level);
  void vSetLedAllOff();
  void vSetLedAllOn(uint8_t level = 128);
  void vTestLed();

  void vUpdateLed();

};

#endif