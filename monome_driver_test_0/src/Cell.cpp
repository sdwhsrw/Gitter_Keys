#include "Cell.h"

Cell::Cell(uint8_t address, TwoWire *i2c, uint8_t sizeX, uint8_t sizeY) : 
p_iAddress(address), p_pI2C(i2c), p_iSizeX(sizeX), p_iSizeY(sizeY), p_iElementNum(sizeX*sizeY)
{
  p_aKeyAction = new uint8_t[p_iElementNum];
  p_aLedLevel = new uint8_t[p_iElementNum];
  for (uint8_t i=0; i<p_iElementNum; i++) p_aKeyAction[i] = 0;
  for (uint8_t i=0; i<p_iElementNum; i++) p_aLedLevel[i] = 0;

  resetKeyEvent();

  // vTestLed(); // this will make pico die!
}

Cell::~Cell()
{
  delete[] p_aKeyAction;
  delete[] p_aLedLevel;
}

/*-----------------------------------------------------------------*/

void Cell::resetKeyEvent()
{
  for (uint8_t i=0; i<KEY_EVENT_MAXNUM; i++)
  {
      p_iKeyEvent[i][0] = CELL_KEY_NOCHANGE;
      p_iKeyEvent[i][1] = 0;
      p_iKeyEvent[i][2] = 0;
  }
}

void Cell::vGetKey()
{ 
  uint8_t eventIndex = 0;
  uint8_t newAction = 0;

  p_pI2C->requestFrom(p_iAddress, 16);
  if (p_pI2C->available())
  {
    for (uint8_t i=0; i<p_iElementNum; i++)
    {
      newAction = p_pI2C->read();
      if ( newAction != p_aKeyAction[i])
      {
        p_aKeyAction[i] = newAction;

        p_iKeyEvent[eventIndex][0] = newAction;
        p_iKeyEvent[eventIndex][1] = i%p_iSizeX;
        p_iKeyEvent[eventIndex][2] = i/p_iSizeX;
        eventIndex++;

        p_iKeyEventFlag = 1;
      }
    }
    while (p_pI2C->available());
  }
}

/*-----------------------------------------------------------------*/


void Cell::vTestLed()
{
  p_pI2C->beginTransmission(p_iAddress);
  for (uint8_t i=0; i<p_iElementNum; i++) p_pI2C->write(128);
  p_pI2C->endTransmission();
  delay(1000);
  p_pI2C->beginTransmission(p_iAddress);
  for (uint8_t i=0; i<p_iElementNum; i++) p_pI2C->write(0);
  p_pI2C->endTransmission();
  delay(500);
  
}

void Cell::vSetLed(uint8_t x, uint8_t y, uint8_t level)
{ 
  if (x >= p_iSizeX || y >= p_iSizeY) return;
  uint8_t index = y*p_iSizeX+x;
  p_aLedLevel[index] = level;

  p_bLedUpdateFlag = 1;
}

void Cell::vSetLed(uint8_t index, uint8_t level)
{
  p_aLedLevel[index] = level;

  p_bLedUpdateFlag = 1;
}

void Cell::vUpdateLed()
{ 
  if (p_bLedUpdateFlag)
  {
    p_pI2C->beginTransmission(p_iAddress);
    for (uint8_t i=0; i<p_iElementNum; i++)
    {
      // p_pI2C->write(i);
      p_pI2C->write(p_aLedLevel[i]);
    }
    p_pI2C->endTransmission();  

    p_bLedUpdateFlag = 0;
  }
}

void Cell::vSetLedAllOn(uint8_t level)
{
  for (uint8_t i=0; i<p_iElementNum; i++) p_aLedLevel[i] = level;
  p_bLedUpdateFlag = 1;
}

void Cell::vSetLedAllOff()
{
  for (uint8_t i=0; i<p_iElementNum; i++) p_aLedLevel[i] = 0;
  p_bLedUpdateFlag = 1;
}

