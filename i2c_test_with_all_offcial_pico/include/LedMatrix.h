#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_

#include "Arduino.h"
#include <RP2040_PWM.h>

#define ROW_SIZE 4
#define COL_SIZE 4
#define PMW_FREQ 2000

class LedMatrix
{
    private:
        const static uint8_t ledCounts = ROW_SIZE * COL_SIZE; 
        uint16_t maxLevel; 
        const uint8_t ledPins[ledCounts] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        uint8_t ledBrightness[ledCounts];
        RP2040_PWM* PWM_Instance[ledCounts];
        
    public:
        LedMatrix();
        void openAllLed();
        void resetLed();
        void testLed();
        bool updateLeds();
        void setLedBrightness(int, int, int);
        void setLedBrightness(int, int);
};

#endif