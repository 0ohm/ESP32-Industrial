/*
  Esp32Industrial.h - Library for controlling ESP32 industrial board.
  Created by Gaston Wlack, January 26, 2019 for 0OHM ingenieria y desarrollo electronico.
  Released into the public domain.
*/

#ifndef Esp32Industrial_h
#define Esp32Industrial_h
 
#include "Arduino.h"
#include <Wire.h>
#include <driver/adc.h>
 
class Esp32Ind
{
  public:
    const int AN1  = 35;
    const int AN2  = 34;
    const int PIN1 = 26;
    const int PIN2 = 25;
    Esp32Ind(long baud);
    Esp32Ind(long baud, int GPIO5f);
    Esp32Ind(long baud, int GPIO5f, int GPIO6f);
    int buttonPressed(void);
    void ledState(int state);
    void relay(int relay, int state);
    float getCurrent(unsigned int frequency, int maxAmp, char sensorType);
    int getOpto(void);
    float getTemp(void);
    float getHum(void);
    int saveData(unsigned int dir, byte data);
    int getData(int dir);
    float getAnalog1(void);
    float getAnalog2(void);

  private:
    void _HDC1080_Start();
    const int _button         = 0;
    const int _LED            = 2;
    const int _REL1           = 5;
    const int _REL2           = 17;
    const int _REL3           = 16;
    const int _currentPin     = 32;  //ADC1_CH4
    const int _opto1          = 13;
    const int _opto2          = 15;
    const int _SDApin         = 19;
    const int _SCLpin         = 18;
    const int _hdc1080Address = 0x40;
    const int _eepromAddress  = 0x54;
};
 
#endif