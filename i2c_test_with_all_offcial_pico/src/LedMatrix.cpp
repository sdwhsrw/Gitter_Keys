#include "LedMatrix.h"

LedMatrix::LedMatrix()
{   
    for (int i=0; i<ledCounts; i++)
    {
        PWM_Instance[i] = new RP2040_PWM(ledPins[i], PMW_FREQ, 0);
        PWM_Instance[i]->setPWM();
    }
    
    maxLevel = PWM_Instance[0]->get_TOP()/128;
    
    resetLed();
}

void LedMatrix::testLed()
{
    openAllLed();
    updateLeds();
    delay(1500);
    resetLed();
    updateLeds();
    delay(500);
}

void LedMatrix::openAllLed()
{
    for (int i=0; i<ledCounts; i++) ledBrightness[i] = 90;
}

void LedMatrix::resetLed()
{
    for (int i=0; i<ledCounts; i++) ledBrightness[i] = 0;
}

void LedMatrix::setLedBrightness(int posX, int posY, int level)
{   
    uint8_t index = posX + ROW_SIZE*posY; 
    // PWM_Instance[i]->setPWM_manual_Fast(ledPins[index], dutyCycle);
    
    ledBrightness[index] = level;
}

void LedMatrix::setLedBrightness(int index, int level)
{
    ledBrightness[index] = level;
}

bool LedMatrix::updateLeds()
{
    for (int i=0; i<ledCounts; i++)
    {
        uint16_t level = ledBrightness[i]*maxLevel;
        PWM_Instance[i]->setPWM_manual_Fast(ledPins[i], level);
    }
    return true;
}