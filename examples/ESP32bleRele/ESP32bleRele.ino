#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Esp32Industrial.h>

Esp32Ind miPlaca(9600, OUTPUT, OUTPUT);

////////////////////////////
/////////BLE Things/////////
////////////////////////////
#define SERVICE_UUID_RELAYS               "ABCD0000-0000-0000-0000-000000000000"
#define CHARACTERISTIC_UUID_RELAY1        "ABCD0000-0001-0000-0000-000000000000"
#define CHARACTERISTIC_UUID_RELAY2        "ABCD0000-0002-0000-0000-000000000000"
#define CHARACTERISTIC_UUID_RELAY3        "ABCD0000-0003-0000-0000-000000000000"

#define SERVICE_UUID_SENSORS              "ABCD0001-0000-0000-0000-000000000000"
#define CHARACTERISTIC_UUID_CURRENT       "ABCD0001-0001-0000-0000-000000000000"
#define CHARACTERISTIC_UUID_TEMPERATURE   "ABCD0001-0002-0000-0000-000000000000"
#define CHARACTERISTIC_UUID_HUMIDITY      "ABCD0001-0003-0000-0000-000000000000"
BLECharacteristic *relay1Charasteristic, *relay2Charasteristic, *relay3Charasteristic, *currentSensorCharasteristic, *temperatureSensorCharasteristic, *humiditySensorCharasteristic;
////////////////////////////
/////End of BLE Things//////
////////////////////////////

std::string rele1;
std::string rele2;
std::string rele3;

float corriente, temperatura, humedad;


////////////////////////////
////////BLE Callback////////
////////////////////////////
class MyCallbacks: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharasteristic)
  {
    //Controlling Relay from BLE
    if(pCharasteristic == relay1Charasteristic)
    {
      rele1 = relay1Charasteristic->getValue();
      if (rele1.length() > 0)
      {
        Serial.print("El estado del Rele 1 es: ");Serial.write(rele1[0]+0x30);Serial.println("");
        miPlaca.relay(1, rele1[0]);
      }
    }
    if(pCharasteristic == relay2Charasteristic)
    {
      rele2 = relay2Charasteristic->getValue();
      if (rele2.length() > 0)
      {
        Serial.print("El estado del Rele 2 es: ");Serial.write(rele2[0]+0x30);Serial.println("");
        miPlaca.relay(2, rele2[0]);
      }
    }
    if(pCharasteristic == relay3Charasteristic)
    {
      rele3 = relay3Charasteristic->getValue();
      if (rele3.length() > 0)
      {
        Serial.print("El estado del Rele 3 es: ");Serial.write(rele3[0]+0x30);Serial.println("");
        miPlaca.relay(3, rele3[0]);
      }
    }
  }
};

void setup()
{
  Serial.println("Comenzando la configuración bluetooth");

  ////////////////////////////
  /////////BLE Things/////////
  ////////////////////////////
  //Create BLE Device
  BLEDevice::init("0OHM ESP32 Industrial");

  //Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  //Create BLE Services
  BLEService *pServiceRelay = pServer->createService(SERVICE_UUID_RELAYS);
  BLEService *pServiceSensors = pServer->createService(SERVICE_UUID_SENSORS);

  //Create BLE Characteristics
  relay1Charasteristic = pServiceRelay->createCharacteristic(
                                         CHARACTERISTIC_UUID_RELAY1,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  relay2Charasteristic = pServiceRelay->createCharacteristic(
                                         CHARACTERISTIC_UUID_RELAY2,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  relay3Charasteristic = pServiceRelay->createCharacteristic(
                                         CHARACTERISTIC_UUID_RELAY3,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  currentSensorCharasteristic = pServiceSensors->createCharacteristic(
                                         CHARACTERISTIC_UUID_CURRENT,
                                         BLECharacteristic::PROPERTY_READ  |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  temperatureSensorCharasteristic = pServiceSensors->createCharacteristic(
                                         CHARACTERISTIC_UUID_TEMPERATURE,
                                         BLECharacteristic::PROPERTY_READ  |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  humiditySensorCharasteristic = pServiceSensors->createCharacteristic(
                                         CHARACTERISTIC_UUID_HUMIDITY,
                                         BLECharacteristic::PROPERTY_READ  |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  
  //Seting callbacks to BLE controlling
  relay1Charasteristic->setCallbacks(new MyCallbacks());
  relay2Charasteristic->setCallbacks(new MyCallbacks());
  relay3Charasteristic->setCallbacks(new MyCallbacks());

  //Start Service
  pServiceRelay->start();
  pServiceSensors->start();

  //Start Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_RELAYS);
  pAdvertising->addServiceUUID(SERVICE_UUID_SENSORS);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  ////////////////////////////
  /////End of BLE Things//////
  ////////////////////////////

  //Blinking LED
  for(int i=0;i<15;i++)
  {
    miPlaca.ledState(LOW);
    delay(30);
    miPlaca.ledState(HIGH);
    delay(30);
  }

  //Setting GPIO1 and GPIO2 (1 and 2) LOW
  digitalWrite(miPlaca.PIN1, LOW);
  digitalWrite(miPlaca.PIN2, LOW);
}

void loop()
{
  ////////////////////////////
  ////Get data from sensor////
  ////////////////////////////
  float temperatura, humedad;
  temperatura = miPlaca.getTemp();
  humedad = miPlaca.getHum();
  //Write to BLE database
  char temperaturaS[10]={0};
  sprintf(temperaturaS, "%f", temperatura);
  temperatureSensorCharasteristic->setValue(temperaturaS);
  temperatureSensorCharasteristic->notify();
  char humedadS[10]={0};
  sprintf(humedadS, "%f", humedad);
  humiditySensorCharasteristic->setValue(humedadS);
  humiditySensorCharasteristic->notify();
  Serial.print("La temperatura es: "); Serial.println(temperatura);
  Serial.print("La Humedad es: "); Serial.println(humedad);

  ///////////////////////////
  ////Save data on EEPROM////
  ///////////////////////////
  int address = 0x0003;
  miPlaca.saveData(address, 0x1A);

  ////////////////////////////
  ////Get data from EEPROM////
  ////////////////////////////
  int dato;
  dato = miPlaca.getData(address);
  Serial.print("El dato guardado en memoria es: ");Serial.println(dato);

  ////////////////////////////
  ////Get the optos states////
  ////////////////////////////
  int estadoOpto;
  estadoOpto = miPlaca.getOpto();
  Serial.print("Opto 1: ");Serial.println(estadoOpto>>1);
  Serial.print("Opto 2: ");Serial.println(estadoOpto&0x01);

  ///////////////////////////////////
  ////Get the current sensor data////
  ///////////////////////////////////
  float corriente;
  corriente = miPlaca.getCurrent(50, 30, 'V');
  //Write to BLE database
  char corrienteS[10]={0};
  sprintf(corrienteS, "%f", corriente);
  currentSensorCharasteristic->setValue(corrienteS);
  currentSensorCharasteristic->notify();
  Serial.print("La corriente es: ");Serial.println(corriente);

  /////////////////////////
  ////A button function////
  /////////////////////////
  if(miPlaca.buttonPressed())
  {
    miPlaca.relay(1, HIGH);
    miPlaca.relay(2, LOW);
    miPlaca.relay(3, LOW);
    delay(200);
    miPlaca.relay(1, LOW);
    miPlaca.relay(2, HIGH);
    miPlaca.relay(3, LOW);
    delay(200);
    miPlaca.relay(1, LOW);
    miPlaca.relay(2, LOW);
    miPlaca.relay(3, HIGH);
    delay(200);
    miPlaca.relay(1, LOW);
    miPlaca.relay(2, LOW);
    miPlaca.relay(3, LOW);
    delay(700);  
    digitalWrite(miPlaca.PIN1, HIGH);
    digitalWrite(miPlaca.PIN2, HIGH);
    delay(2000);  
    digitalWrite(miPlaca.PIN1, LOW);
    digitalWrite(miPlaca.PIN2, LOW);
    Serial.println(miPlaca.getAnalog1());
    Serial.println(miPlaca.getAnalog2());
  }

  Serial.println("___________________________");Serial.println("");  
  delay(700);
}
