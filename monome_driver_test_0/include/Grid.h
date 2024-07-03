#ifndef GRID_H
#define GRID_H

#include <Arduino.h>

class Cell;

class Grid
{
private:
  const uint8_t p_iCellRow; // for how many rows of cells
  const uint8_t p_iCellCol; // for how many columns of cells
  const uint8_t p_iCellNum; // for total cells amount 
  Cell *p_pCells; // const? no. 

  const uint8_t p_iCellSize = 4; // how many rows and cols of elements on singel cell

  uint8_t *p_aLedLevel; // really need?

  uint8_t p_iLedUpdateFlag = 0;
  uint8_t p_iKeySendFlag = 0;
  
public:
  const uint8_t p_iElementRow; // for how many rows of elements
  const uint8_t p_iElementCol; // for how many cols of elements
  const uint8_t p_iElementNum; // for total elements amount 

  Grid() = default;
  Grid(Cell *cells, uint8_t cellRow=2, uint8_t cellCol=2);
  ~Grid();

  // key
  void vGetGridKey();
  void vSendGridKey();
  
  // led
  void vSetGridLed(uint8_t index, uint8_t level);
  void vSetGridLed(uint8_t posX, uint8_t posY, uint8_t level);
  void vSetAllLedOn();
  void vSetAllLedOff();

  void vUpdateGridLed();

  void vGridLedTest();

  

};

#endif