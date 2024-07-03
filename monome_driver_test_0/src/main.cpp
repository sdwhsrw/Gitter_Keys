#include <Arduino.h>
#include <Wire.h>
#include <elapsedMillis.h>
#include <Adafruit_TinyUSB.h> 

#include "Grid.h"
#include "Cell.h"
#include "MonomeSerialDevice.h"

#define NUM_ROWS 8   // DIM_Y number of rows of keys down
#define NUM_COLS 8  // DIM_X number of columns of keys across
#define NUM_LEDS NUM_ROWS* NUM_COLS

#define I2C_SDA 8
#define I2C_SCL 9

void sendLeds();

const uint8_t gammaTable[16] = { 0, 2, 3, 6, 11, 18, 25, 32, 41, 59, 70, 80, 92, 103, 115, 128 };

bool isInited = false;
elapsedMillis monomeRefresh; // timer

// set your monome device name here
String deviceID = "neo-monome";
String serialNum = "m4216124";

// DEVICE INFO FOR TinyUSB
char mfgstr[32] = "monome";
char prodstr[32] = "monome";
char serialstr[32] = "m4216124";

// Monome class setup
MonomeSerialDevice mdp;
Cell cells[NUM_ROWS / 4][NUM_COLS / 4] = {{Cell(0x01, &Wire), Cell(0x02, &Wire)},
                                          {Cell(0x03, &Wire), Cell(0x04, &Wire)}};
Grid g1((Cell *)cells, NUM_COLS / 4, NUM_COLS / 4); // 顺序问题？？？？？？？？？？？？？？？？？？？

int prevLedBuffer[NUM_COLS*NUM_ROWS];



void setup() 
{
  // g1.vSetAllLedOn();
  // g1.vUpdateGridLed();
  // delay(500);
  // g1.vSetAllLedOff();
  // g1.vUpdateGridLed();
  // delay(500);

  uint8_t x, y;

  // 基础设置 保留
  USBDevice.setManufacturerDescriptor(mfgstr);
  USBDevice.setProductDescriptor(prodstr);
  USBDevice.setSerialDescriptor(serialstr);

  Serial.begin(115200);

  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();

  mdp.isMonome = true;
  mdp.deviceID = deviceID;
  mdp.setupAsGrid(NUM_ROWS, NUM_COLS);
  monomeRefresh = 0; //elapsedMillis对象 重置计时器
  isInited = true;

  int var = 0;
  // usb接收并处理8次
  while (var < 8) 
  {
    mdp.poll();
    var++;
    delay(100);
  }

  Serial.println("set up finished");

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
  for (int i = 0; i < NUM_ROWS * NUM_COLS; i++)
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

