#include "MonomeSerialDevice.h"
#include "debug.h"
#include "tusb.h"

MonomeSerialDevice::MonomeSerialDevice() {}

void MonomeSerialDevice::initialize() {
    active = false;
    isMonome = false;
    isGrid = true;
    rows = 0;
    columns = 0;
    encoders = 0;
    //clearQueue();
    clearAllLeds();
    arcDirty = false;
    gridDirty = false;
}

void MonomeSerialDevice::setupAsGrid(uint8_t _rows, uint8_t _columns) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = true;
    rows = _rows;
    columns = _columns;
    gridDirty = true;
    debugfln(INFO, "GRID rows: %d columns %d", rows, columns);
}

void MonomeSerialDevice::setupAsArc(uint8_t _encoders) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = false;
    encoders = _encoders;
    arcDirty = true;
    debugfln(INFO, "ARC encoders: %d", encoders);
}

void MonomeSerialDevice::getDeviceInfo() {
    if (tud_cdc_connected()) {
      uint8_t zero = 0;
      tud_cdc_write(&zero, 1);
      tud_cdc_write_flush();
    }
}

void MonomeSerialDevice::poll() {
    if (tud_cdc_available()) {
      processSerial();
    }
}


void MonomeSerialDevice::setAllLEDs(int value) {
  for (int i = 0; i < MAXLEDCOUNT; i++) leds[i] = value;
}

void MonomeSerialDevice::setGridLed(uint8_t x, uint8_t y, uint8_t level) {
    if (x < columns && y < rows) {
      uint32_t index = y * columns + x;
      leds[index] = level;
    }
}
        
void MonomeSerialDevice::clearGridLed(uint8_t x, uint8_t y) {
    setGridLed(x, y, 0);
}

void MonomeSerialDevice::setArcLed(uint8_t enc, uint8_t led, uint8_t level) {
    int index = led + (enc << 6);
    if (index < MAXLEDCOUNT) leds[index] = level;
}
        
void MonomeSerialDevice::clearArcLed(uint8_t enc, uint8_t led) {
    setArcLed(enc, led, 0);
}

void MonomeSerialDevice::clearAllLeds() {
    for (int i = 0; i < MAXLEDCOUNT; i++) leds[i] = 0;
}

void MonomeSerialDevice::clearArcRing(uint8_t ring) {
    for (int i = ring << 6, upper = i + 64; i < upper; i++) leds[i] = 0;
}

void MonomeSerialDevice::refreshGrid() {
    gridDirty = true;
}

void MonomeSerialDevice::refreshArc() {
    arcDirty = true;
}

void MonomeSerialDevice::refresh() {
}

void MonomeSerialDevice::processSerial() {
    uint8_t identifierSent;  // command byte sent from controller to matrix
    uint8_t index, readX, readY, readN, readA;
    uint8_t dummy, gridNum, deviceAddress;  // for reading in data not used by the matrix
    uint8_t n, x, y, z, i;
    uint8_t intensity = 15;
    uint8_t gridKeyX;
    uint8_t gridKeyY;
    int8_t delta;
    uint8_t gridX    = columns;          // Will be either 8 or 16
    uint8_t gridY    = rows;
    uint8_t numQuads = columns/rows;
    
    // get command identifier: first byte of packet is identifier in the form: [(a << 4) + b]
    // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
    // b = command (ie. query, enable, led, key, frame)

    tud_cdc_read(&identifierSent, 1);  // read first byte
    
    switch (identifierSent) {
        case 0x00:  // device information
        	// [null, "led-grid", "key-grid", "digital-out", "digital-in", "encoder", "analog-in", "analog-out", "tilt", "led-ring"]
            {
              uint8_t resp0[] = {0x00, 0x01, numQuads};
              tud_cdc_write(resp0, 3);
              uint8_t resp1[] = {0x00, 0x02, numQuads};
              tud_cdc_write(resp1, 3);
              tud_cdc_write_flush();
            }
            break;

        case 0x01:  // system / ID
            {
              uint8_t resp = 0x01;
              tud_cdc_write(&resp, 1);
              for (i = 0; i < 32; i++) {          // has to be 32
                  if (i < deviceID.length()) {
                    tud_cdc_write(&deviceID[i], 1);
                  } else {
                    uint8_t zero = 0;
                    tud_cdc_write(&zero, 1);
                  }
              }
              tud_cdc_write_flush();
            }
            break;

        case 0x02:  // system / write ID
            while (tud_cdc_available() < 32) {
              tud_task();
              delay(1); 
            }
            for (int i = 0; i < 32; i++) {  // has to be 32
              uint8_t b;
              tud_cdc_read(&b, 1);
              deviceID[i] = b;
            }
            break;

        case 0x03:  // system / report grid offset
            {
              uint8_t resp[] = {0x02, 0x01, 0, 0};
              tud_cdc_write(resp, 4);
              tud_cdc_write_flush();
            }
            break;

        case 0x04:  // system / report ADDR
            tud_cdc_read(&gridNum, 1);
            tud_cdc_read(&readX, 1);
            tud_cdc_read(&readY, 1);
            break;

        case 0x05:  // _SYS_GET_GRID_SIZE
            {
              uint8_t resp[] = {0x03, gridX, gridY};
              tud_cdc_write(resp, 3);
              tud_cdc_write_flush();
            }
            break;

        case 0x06:
            tud_cdc_read(&readX, 1);
            tud_cdc_read(&readY, 1);
            break;

        case 0x07:
            break;                              // I2C get addr (scan) - ignored

        case 0x08:
            tud_cdc_read(&deviceAddress, 1);
            tud_cdc_read(&dummy, 1);
            break;


        case 0x0F:  // system / report firmware version
            for (int i = 0; i < 8; i++) {  // 8 character string
            }
            break;


      // 0x10-0x1F are for an LED Grid Control.  All bytes incoming, no responses back
  
        case 0x10:            // /prefix/led/set x y [0/1]  / led off
          tud_cdc_read(&readX, 1);
          tud_cdc_read(&readY, 1);
          setGridLed(readX, readY, 0);
          break;

        case 0x11:            // /prefix/led/set x y [0/1]   / led on
          tud_cdc_read(&readX, 1);
          tud_cdc_read(&readY, 1);
          setGridLed(readX, readY, 15);   // need global brightness variable?
          break;

        case 0x12:            //  /prefix/led/all [0/1]  / all off
          clearAllLeds();
          break;

        case 0x13:                      //  /prefix/led/all [0/1] / all on
          setAllLEDs(15);
          break;

        case 0x14:                  // /prefix/led/map x y d[8]  / map (frame)
          tud_cdc_read(&readX, 1);
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8

          tud_cdc_read(&readY, 1);                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8

          for (y = 0; y < 8; y++) {               // each i will be a row
            tud_cdc_read(&intensity, 1);            // read one byte of 8 bits on/off
    
            for (x = 0; x < 8; x++) {             // for 8 LEDs on a row
              if ((intensity >> x) & 0x01) {      // if intensity bit set, light led full brightness
                setGridLed(readX + x, readY + y, 15); 
              }
              else {
                setGridLed(readX + x, readY + y, 0); 
              }
            }
          }
          break;

        case 0x15:                                //  /prefix/led/row x y d
          tud_cdc_read(&readX, 1);                      // led-grid / set row
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8

          tud_cdc_read(&readY, 1);                      // 
          tud_cdc_read(&intensity, 1);                  // read one byte of 8 bits on/off

          for (x = 0; x < 8; x++) {               // for the next 8 lights in row
            if ((intensity >> x) & 0x01) {        // if intensity bit set, light led full brightness
              setGridLed(readX + x, readY, 15);
            } else {
              setGridLed(readX + x, readY, 0);
            }
          }

          break;

        case 0x16:                                //  /prefix/led/col x y d
          tud_cdc_read(&readX, 1);                      // led-grid / column set

          tud_cdc_read(&readY, 1);
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8

          tud_cdc_read(&intensity, 1);                  // read one byte of 8 bits on/off

          for (y = 0; y < 8; y++) {               // for the next 8 lights in column
            if ((intensity >> y) & 0x01) {        // if intensity bit set, light led full brightness
              setGridLed(readX, readY + y, 15);
            } else {
              setGridLed(readX, readY + y, 0);
            }
          }

          break;

        case 0x17:                                     //  /prefix/led/intensity i
          tud_cdc_read(&intensity, 1);                      // set brightness for entire grid
          setAllLEDs(intensity);
          break;

        case 0x18:                                //  /prefix/led/level/set x y i
          tud_cdc_read(&readX, 1);                      // led-grid / set LED intensity
          tud_cdc_read(&readY, 1);                      // read the x and y coordinates
          tud_cdc_read(&intensity, 1);                  // read the intensity
          setGridLed(readX, readY, intensity);              
          break;

        case 0x19:                               //  /prefix/led/level/all s
          tud_cdc_read(&intensity, 1);                 // set all leds
          setAllLEDs(intensity);              
          break;

        case 0x1A:                               //   /prefix/led/level/map x y d[64]
          tud_cdc_read(&readX, 1);                      // x offset
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8
          tud_cdc_read(&readY, 1);                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8
          
          z = 0;
          for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
              if (z % 2 == 0) {                    
                tud_cdc_read(&intensity, 1);
                if ( ((intensity >> 4) & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
                  setGridLed(readX + x, readY + y, (intensity >> 4) & 0x0F);
                } else {
                  setGridLed(readX + x, readY + y, 0);
                }
              } else { 
                if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
                  setGridLed(readX + x, readY + y, intensity & 0x0F);
                } else {
                  setGridLed(readX + x, readY + y, 0);
                }
              }
              z++;
            }
          }
          break;

        case 0x1B:                                // /prefix/led/level/row x y d[8]
          tud_cdc_read(&readX, 1);                      // x offset
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8
          tud_cdc_read(&readY, 1);                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8
          for (x = 0; x < 8; x++) {
              if (x % 2 == 0) {                    
                tud_cdc_read(&intensity, 1);
                if ( (intensity >> 4 & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
                  setGridLed(readX + x, readY, (intensity >> 4) & 0x0F);
                }
                else {
                  setGridLed(readX + x, readY, 0);
                }
              } else {                              
                if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
                  setGridLed(readX + x, readY, intensity & 0x0F);
                }
                else {
                  setGridLed(readX + x, readY, 0);
                }
              }
          }
          break;

        case 0x1C:                                // /prefix/led/level/col x y d[8]
          tud_cdc_read(&readX, 1);                      // x offset
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8
          tud_cdc_read(&readY, 1);                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8
          for (y = 0; y < 8; y++) {
              if (y % 2 == 0) {                    
                tud_cdc_read(&intensity, 1);
                if ( (intensity >> 4 & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
                  setGridLed(readX, readY + y, (intensity >> 4) & 0x0F);
                }
                else {
                  setGridLed(readX, readY + y, 0);
                }
              } else {                              
                if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
                  setGridLed(readX, readY + y, intensity & 0x0F);
                }
                else {
                  setGridLed(readX, readY + y, 0);
                }
              }
          }
          break;

        case 0x20:
            tud_cdc_read(&gridKeyX, 1);
            tud_cdc_read(&gridKeyY, 1);
            addGridEvent(gridKeyX, gridKeyY, 0);
            break;
            
        case 0x21:
            tud_cdc_read(&gridKeyX, 1);
            tud_cdc_read(&gridKeyY, 1);
            addGridEvent(gridKeyX, gridKeyY, 1);
            break;

        case 0x50:
            tud_cdc_read(&index, 1);
            {
              uint8_t d;
              tud_cdc_read(&d, 1);
              delta = (int8_t)d;
            }
            addArcEvent(index, delta);
            break;

        case 0x51:  // /prefix/enc/key n (key up)
            tud_cdc_read(&n, 1);
            break;

        case 0x52:  // /prefix/enc/key n (key down)
            tud_cdc_read(&n, 1);
            break;

        case 0x80:  //   tilt / active response - 9 bytes [0x01, d]
            break;
        case 0x81:  //   tilt - 8 bytes [0x80, n, xh, xl, yh, yl, zh, zl]
            break;

        case 0x90:
          tud_cdc_read(&readN, 1);
          tud_cdc_read(&readX, 1);
          tud_cdc_read(&readA, 1);
          setArcLed(readN, readX, readA);         
          break;
     
        case 0x91:
          tud_cdc_read(&readN, 1);
          tud_cdc_read(&readA, 1);
          for (int q=0; q<64; q++){
            setArcLed(readN, q, readA);
          }
          break;
      
        case 0x92:
          tud_cdc_read(&readN, 1);
          for (y = 0; y < 64; y++) {
              if (y % 2 == 0) {                    
                tud_cdc_read(&intensity, 1);
                if ( (intensity >> 4 & 0x0F) > 0) {  // even bytes, use upper nybble
                  setArcLed(readN, y, (intensity >> 4 & 0x0F)); 
                }
                else {
                  setArcLed(readN, y, 0);   
                }
              } else {                              
                if ((intensity & 0x0F) > 0 ) {      // odd bytes, use lower nybble
                  setArcLed(readN, y, intensity & 0x0F);
                }
                else {
                  setArcLed(readN, y, 0);
                }
              }
          }
          break;

        case 0x93:
          tud_cdc_read(&readN, 1);
          tud_cdc_read(&readX, 1);  // x1
          tud_cdc_read(&readY, 1);  // x2
          tud_cdc_read(&readA, 1);
      
          if (readX < readY){
            for (y = readX; y < readY; y++) {
              setArcLed(readN, y, readA);
            }
          }else{
            for (y = readX; y < 64; y++) {
              setArcLed(readN, y, readA);
            }
            for (x = 0; x < readY; x++) {
              setArcLed(readN, y, readA);
            }
          }
         break;

        default: 
          break;
    }
}

void MonomeEventQueue::addGridEvent(uint8_t x, uint8_t y, uint8_t pressed) {
    if (gridEventCount >= MAXEVENTCOUNT) return;
    uint8_t ind = (gridFirstEvent + gridEventCount) % MAXEVENTCOUNT;
    gridEvents[ind].x = x;
    gridEvents[ind].y = y;
    gridEvents[ind].pressed = pressed;
    gridEventCount++;
}

bool MonomeEventQueue::gridEventAvailable() {
    return gridEventCount > 0;
}

MonomeGridEvent MonomeEventQueue::readGridEvent() {
    if (gridEventCount == 0) return emptyGridEvent;
    gridEventCount--;
    uint8_t index = gridFirstEvent;
    gridFirstEvent = (gridFirstEvent + 1) % MAXEVENTCOUNT;
    return gridEvents[index];
}

void MonomeEventQueue::addArcEvent(uint8_t index, int8_t delta) {
    if (arcEventCount >= MAXEVENTCOUNT) return;
    uint8_t ind = (arcFirstEvent + arcEventCount) % MAXEVENTCOUNT;
    arcEvents[ind].index = index;
    arcEvents[ind].delta = delta;
    arcEventCount++;
}

bool MonomeEventQueue::arcEventAvailable() {
    return arcEventCount > 0;
}

MonomeArcEvent MonomeEventQueue::readArcEvent() {
    if (arcEventCount == 0) return emptyArcEvent;
    arcEventCount--;
    uint8_t index = arcFirstEvent;
    arcFirstEvent = (arcFirstEvent + 1) % MAXEVENTCOUNT;
    return arcEvents[index];
}

void MonomeEventQueue::sendArcDelta(uint8_t index, int8_t delta) {
    if (tud_cdc_connected()) {
      uint8_t buf[] = {0x50, index, (uint8_t)delta};
      tud_cdc_write(buf, 3);
      tud_cdc_write_flush();
    }
}

void MonomeEventQueue::sendArcKey(uint8_t index, uint8_t pressed) {
    if (tud_cdc_connected()) {
      uint8_t buf[2];
      if (pressed == 1){
        buf[0] = 0x52;
      }else{
        buf[0] = 0x51;
      }
      buf[1] = index;
      tud_cdc_write(buf, 2);
      tud_cdc_write_flush();
    }
}

void MonomeEventQueue::sendGridKey(uint8_t x, uint8_t y, uint8_t pressed) {    
    if (tud_cdc_connected()) {
      uint8_t id = (pressed == 1) ? 0x21 : 0x20;
      uint8_t buf[] = {id, x, y};
      tud_cdc_write(buf, 3);
      tud_cdc_write_flush();
    }
}
