#include <Arduino.h>
#include <Wire.h>
#include <elapsedMillis.h>
#include <tusb.h>

#include "Grid.h"
#include "Cell.h"
#include "MonomeSerialDevice.h"

// usb config 
#include "custom_tusb_config.h"

#define I2C_SDA 8
#define I2C_SCL 9

void sendLeds();

const uint8_t gammaTable[16] = { 0, 2, 3, 6, 11, 18, 25, 32, 41, 59, 70, 80, 92, 103, 115, 128 };

bool isInited = false;
elapsedMillis monomeRefresh; // timer

// set your monome device name here
String deviceID = "neo-monome";
String serialNum = "m4216124";

// Led / Cell Info
constexpr uint8_t ledRows = 8;
constexpr uint8_t ledCols = 8;
constexpr uint8_t cellSize = 4;
constexpr uint8_t cellRows = ledRows / cellSize;
constexpr uint8_t cellCols = ledCols / cellSize;
constexpr uint16_t ledNum = ledRows * ledCols;
constexpr uint16_t cellNum = cellRows * cellCols;

// Monome class setup
MonomeSerialDevice mdp;
Cell cells[cellRows][cellCols] = {{Cell(0x01, &Wire), Cell(0x02, &Wire)},
                                  {Cell(0x03, &Wire), Cell(0x04, &Wire)}};
Grid g1((Cell *)cells, cellCols, cellRows); 

int prevLedBuffer[ledNum];



void setup() 
{
  // I2C setup  
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
  // Initialize each cell and check if it's there
  for (int i=0; i<cellRows; i++)
  {
    for (int j=0; j<cellCols; j++)
    {
      cells[i][j].begin();
    }
  }

  // USB Setup

  tud_init(BOARD_TUD_RHPORT);

  mdp.isMonome = true;
  mdp.deviceID = deviceID;
  mdp.setupAsGrid(ledRows, ledCols);
  monomeRefresh = 0; //elapsedMillis对象 重置计时器
  isInited = true;

  int var = 0;
  //? wait for ready ?
  while (var < 8) 
  {
    tud_task(); //? for what ? 
    mdp.poll();
    var++;
    delay(100);
  }

  // Serial.println("set up finished");

  // test and reset
  mdp.setAllLEDs(15);
  sendLeds();
  delay(500);
  mdp.setAllLEDs(0);
  sendLeds();

  g1.vSetAllLedOn();
  delay(500);
  g1.vSetAllLedOff();
  delay(500);
}

void loop() 
{
  tud_task();
  mdp.poll();

  if (isInited && monomeRefresh > 16)
  {
    g1.vGetGridKey();
    g1.vSendGridKey();
    sendLeds();
    monomeRefresh = 0;
  }
}

void sendLeds()
{
  uint8_t value, prevValue = 0;
  // bool updateFlag = false;
  for (int i = 0; i < ledNum; i++)
  {
    value = mdp.leds[i];
    prevValue = prevLedBuffer[i];
    
    if (value != prevValue)
    {
      g1.vSetGridLed(i, gammaTable[value]);
      prevLedBuffer[i] = value;
      // updateFlag = true;
    }
  }
  g1.vUpdateGridLed();
}

