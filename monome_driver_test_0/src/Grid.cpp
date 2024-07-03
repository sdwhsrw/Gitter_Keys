#include "Grid.h"
#include "Cell.h"

Grid::Grid(Cell *cells, uint8_t cellRow, uint8_t cellCol) : 
p_pCells(cells), p_iCellRow(cellRow), p_iCellCol(cellCol), p_iCellNum(cellRow*cellCol),
p_iElementCol(cellCol*p_iCellSize), p_iElementRow(cellRow*p_iCellSize), p_iElementNum(p_iElementCol*p_iElementRow)
{
  // p_aLedLevel = new uint8_t[p_iElementNum];
  // for (uint8_t i=0; i<p_iElementNum; i++) p_aLedLevel[i] = 0;
}

Grid::~Grid()
{
  // delete[] p_aCells;
  // delete[] p_aLedLevel;
}

/*----------------------------------------------------------------------------*/
void Grid::vGetGridKey()
{
  for (uint8_t i=0; i<p_iCellNum; i++)
  {
    (p_pCells+i)->vGetKey();
  }
}

void Grid::vSendGridKey()
{
  for (uint8_t y=0; y<p_iCellRow; y++)
  {
    for (uint8_t x=0; x<p_iCellCol; x++)
    {
      uint8_t index = y*p_iCellCol+x;
      if ((p_pCells+index)->p_iKeyEventFlag)
      {
        for (uint8_t i=0; i<KEY_EVENT_MAXNUM; i++)
        { 
          uint8_t command = 0x00;
          if ((p_pCells+index)->p_iKeyEvent[i][0] == CELL_KEY_NOCHANGE) break;
          else if ((p_pCells+index)->p_iKeyEvent[i][0] == CELL_KEY_RELEASE)
          {
            command = 0x20;
          }
          else if ((p_pCells+index)->p_iKeyEvent[i][0] == CELL_KEY_PRESS)
          {
            command = 0x21;
          }

          uint8_t elementPosX = x*p_iCellSize+((p_pCells+index)->p_iKeyEvent[i][1]);
          uint8_t elementPosY = y*p_iCellSize+((p_pCells+index)->p_iKeyEvent[i][2]);
          Serial.write(command);
          Serial.write(elementPosX);
          Serial.write(elementPosY);
        }
      }
      (p_pCells+index)->resetKeyEvent();
      (p_pCells+index)->p_iKeyEventFlag = 0;
    }
  }
}

/*----------------------------------------------------------------------------*/

void Grid::vSetGridLed(uint8_t index, uint8_t level)
{
  uint8_t posY = index/p_iElementCol;
  uint8_t posX = index%p_iElementCol;

  uint8_t cellX = posX/p_iCellSize;
  uint8_t cellY = posY/p_iCellSize;
  uint8_t cellIndex = cellY*p_iCellCol+cellX; 
  
  uint8_t innerPosX = posX - cellX*p_iCellSize;
  uint8_t innerPosY = posY - cellY*p_iCellSize;
  
  (p_pCells+cellIndex)->vSetLed(innerPosX, innerPosY, level);
  
  p_iLedUpdateFlag = 1;
}

void Grid::vSetGridLed(uint8_t posX, uint8_t posY, uint8_t level)
{
  uint8_t cellX = posX/p_iCellSize;
  uint8_t cellY = posY/p_iCellSize;
  uint8_t cellIndex = cellY*p_iCellCol+cellX; 
  
  uint8_t innerPosX = posX - cellX*p_iCellSize;
  uint8_t innerPosY = posY - cellY*p_iCellSize;
  
  (p_pCells+cellIndex)->vSetLed(innerPosX, innerPosY, level);
  
  p_iLedUpdateFlag = 1;
}

void Grid::vUpdateGridLed()
{ 
  if (p_iLedUpdateFlag == 1)
  {
    for (uint8_t i=0; i<p_iCellNum; i++) (p_pCells+i)->vUpdateLed();
  }
  p_iLedUpdateFlag = 0;
}

void Grid::vSetAllLedOn()
{
  for ( uint8_t c=0; c<p_iCellNum; c++) (p_pCells+c)->vSetLedAllOn();
  p_iLedUpdateFlag = 1;
}
void Grid::vSetAllLedOff()
{
  for ( uint8_t c=0; c<p_iCellNum; c++) (p_pCells+c)->vSetLedAllOff();
  p_iLedUpdateFlag = 1;
}

void Grid::vGridLedTest()
{
  vSetAllLedOn();
  vUpdateGridLed();
  delay(1000);
  vSetAllLedOff();
  vUpdateGridLed();
  delay(1000);
}