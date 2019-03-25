/*
  Esp32Industrial.h - Library for controlling ESP32 industrial board.
  Created by Gaston Wlack, January 26, 2019 for 0OHM ingenieria y desarrollo electronico.
  Released into the public domain.
*/
 
#include "Arduino.h"
#include "Esp32Industrial.h"

 
Esp32Ind::Esp32Ind(long baud)
{
  Serial.begin(baud);
  Wire.begin(_SDApin, _SCLpin);
  _HDC1080_Start();
  pinMode(_button, INPUT);
  pinMode(_opto1, INPUT);
  pinMode(_opto2, INPUT);
  pinMode(_REL1, OUTPUT);
  pinMode(_REL2, OUTPUT);
  pinMode(_REL3, OUTPUT);
  pinMode(_LED, OUTPUT);
  //adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_DB_0);
}

Esp32Ind::Esp32Ind(long baud, int GPIO5f)
{
  Serial.begin(baud);
  Wire.begin(_SDApin, _SCLpin);
  _HDC1080_Start();
  pinMode(_button, INPUT);
  pinMode(_opto1, INPUT);
  pinMode(_opto2, INPUT);
  pinMode(_REL1, OUTPUT);
  pinMode(_REL2, OUTPUT);
  pinMode(_REL3, OUTPUT);
  pinMode(PIN1, GPIO5f);
  pinMode(_LED, OUTPUT);
}

Esp32Ind::Esp32Ind(long baud, int GPIO5f, int GPIO6f)
{
  Serial.begin(baud);
  Wire.begin(_SDApin, _SCLpin);
  _HDC1080_Start();
  pinMode(_button, INPUT);
  pinMode(_opto1, INPUT);
  pinMode(_opto2, INPUT);
  pinMode(_REL1, OUTPUT);
  pinMode(_REL2, OUTPUT);
  pinMode(_REL3, OUTPUT);
  pinMode(PIN1, GPIO5f);
  pinMode(PIN2, GPIO6f);
  pinMode(_LED, OUTPUT);
}

int Esp32Ind::buttonPressed(void)
{
  return !digitalRead(_button);  
}
 
void Esp32Ind::relay(int relay, int state)
{
  switch(relay)
  {
    case 1:
      digitalWrite(_REL1, state);
    break;
    case 2:
      digitalWrite(_REL2, state);
    break;
    case 3:
      digitalWrite(_REL3, state);
    break;
  }
}

int Esp32Ind::getOpto(void)
{
  int a, b;
  a = !digitalRead(_opto1);
  b = !digitalRead(_opto2);
  /*
  *No Opto       0     //No input
  *Opto 1        1     //Only input on Opto 1
  *Opto 2        2     //Only input on Opto 2
  *Opto 1 and 1  3     //Input on both Optos
  */
  return (a<<1)|b;  
}

int Esp32Ind::saveData(unsigned int dir, byte data)
{
  Wire.beginTransmission(_eepromAddress);
  Wire.write((int)(dir >> 8)); // MSB
  Wire.write((int)(dir & 0xFF)); // LSB
  Wire.write(data);
  int error = Wire.endTransmission();
  return error;
}

int Esp32Ind::getData(int dir)
{
  Wire.beginTransmission(_eepromAddress);
  Wire.write((int)(dir >> 8)); // MSB
  Wire.write((int)(dir & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(_eepromAddress,1);
  if (Wire.available())
  {
    return Wire.read();
  }else
  {
    return -1;
  }
}

float Esp32Ind::getTemp(void)
{
  uint32_t data;
  int dataFromHDC1080[4];
  Wire.beginTransmission(_hdc1080Address);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(50);
  Wire.requestFrom(_hdc1080Address,4);
  dataFromHDC1080[0] = Wire.read();
  dataFromHDC1080[1] = Wire.read();
  dataFromHDC1080[2] = Wire.read();
  dataFromHDC1080[3] = Wire.read();

  return ((float)((dataFromHDC1080[0]<<8)|(dataFromHDC1080[1]&0b0000000011111111)))*165/65356-40;
}

float Esp32Ind::getHum(void)
{
  uint32_t data;
  int dataFromHDC1080[4];
  Wire.beginTransmission(_hdc1080Address);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(50);
  Wire.requestFrom(_hdc1080Address,4);
  dataFromHDC1080[0] = Wire.read();
  dataFromHDC1080[1] = Wire.read();
  dataFromHDC1080[2] = Wire.read();
  dataFromHDC1080[3] = Wire.read();

  return ((float)((dataFromHDC1080[2]<<8)|(dataFromHDC1080[3]&0b0000000011111111)))*100/65356;
}

void Esp32Ind::_HDC1080_Start()
{
  Wire.beginTransmission(_hdc1080Address);
  Wire.write(0x02);
  Wire.write(0x10);
  Wire.write(0x00);
  Wire.endTransmission();
}

float Esp32Ind::getCurrent(unsigned int frequency, int maxAmp, char sensorType) //If return -1.0, error on sensorType
{
  float sensorVoltage;
  float currentData=0;
  float sum=0;
  long time=millis();
  int N=0;
  int period = 0;
  period = frequency == 60 ? 500:600;
  
  while(millis()-time<period) //30 periods of senoidal current
  { 
    int val;
    adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &val);
    sensorVoltage = analogRead(_currentPin) * (3.3 / 4095.0); //Converting to volts
    currentData = sensorVoltage*maxAmp/3.3; //30A for each Volt Amplified by 3.3
    if(sensorType == 'c' || sensorType == 'C')
    {
      currentData = currentData/1.1;
    }else if (sensorType != 'c' && sensorType != 'C' && sensorType != 'v' && sensorType != 'V')
    {
      return -1.0;
    }
    sum = sum +sq(currentData);
    N = N + 1;
    delay(1);
  }
  sum=sum*2;//Considering negatives semicycles
  currentData=sqrt((sum)/N); //current in RMS and Amperes [A]
  return currentData;
}

void Esp32Ind::ledState(int state)
{
  switch(state)
  {
    case 0:
      digitalWrite(_LED, LOW);
    break;
    case 1:
      digitalWrite(_LED, HIGH);
    break;
  }
}

float Esp32Ind::getAnalog1(void)
{
  float analogValue;
  analogValue = (float)analogRead(AN1);
  return analogValue;
}

float Esp32Ind::getAnalog2(void)
{
  float analogValue;
  analogValue = (float)analogRead(AN2);
  return analogValue;
}