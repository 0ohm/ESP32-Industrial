#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "esp_wps.h"

#include <Esp32Industrial.h>


Esp32Ind miPlaca(9600, OUTPUT, OUTPUT);

////////////////////////////
/////////Wifi Things////////
////////////////////////////
#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "0OHM ingenieria"
#define ESP_MODEL_NUMBER  "1.0"
#define ESP_MODEL_NAME    "ESP32 Industrial"
#define ESP_DEVICE_NAME   "Nodo de pruebas"

static esp_wps_config_t config;

// Replace the next variables with your SSID/Password if you wont use WPS
const char* ssid = "";
const char* password = "";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
//const char* mqtt_server = "iot.eclipse.org";
char mqtt_server[] = "industrial.api.ubidots.com";
int port = 1883;

char clientIDMQTT[] = "ESP32 Industrial 0OHM"; //Client ID MQTT, ubidots TOKEN
char userNameMQTT[] = "BBFF-xXXHNrDjozUZQ4vnNuClsTiK7OgZAs"; //Nombre de usuario MQTT
char passwordMQTT[] = ""; //Password MQTT
char ubidotsTopic[] = "/v1.6/devices/0ohm-iot"; //Topico ubidots para envío de datos
char ubidotsTopicRele1[] = "/v1.6/devices/0ohm-iot/rele1/lv"; //Topico ubidots para rele1
char ubidotsTopicRele2[] = "/v1.6/devices/0ohm-iot/rele2/lv"; //Topico ubidots para rele2
char ubidotsTopicRele3[] = "/v1.6/devices/0ohm-iot/rele3/lv"; //Topico ubidots para rele3
char payloadTemp[] = "{\"temperatura-ambiental\" :{\"value\":";
char payloadHum[] = "{\"humedad-relativa\" :{\"value\":";
char payloadCurr[] = "{\"corriente\" :{\"value\":";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

////////////////////////////
/////End of Wifi Things/////
////////////////////////////


float corriente, temperatura, humedad;


//////////////////////////////
/////WPS and Wifi Things//////
//////////////////////////////
void wpsInitConfig(){
  config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

String wpspin2string(uint8_t a[]){
  char wps_pin[9];
  for(int i=0;i<8;i++){
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}

void WiFiEvent(WiFiEvent_t event, system_event_info_t info){
  switch(event){
    case SYSTEM_EVENT_STA_START:
      Serial.println("Station Mode Started");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("Connected to :" + String(WiFi.SSID()));
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Disconnected from station, attempting reconnection");
      WiFi.reconnect();
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      esp_wifi_wps_disable();
      delay(10);
      WiFi.begin();
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      Serial.println("WPS Failed, retrying");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      Serial.println("WPS Timedout, retrying");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      Serial.println("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
      break;
    default:
      break;
  }
}

void setup()
{
  Serial.println("Comenzando la configuración Wifi");

  ////////////////////////////
  /////////Wifi Things////////
  ////////////////////////////
  WiFi.reconnect();
  
  //Use only if you wont use WPS
  setup_wifi();
  
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
  ////////////////////////////
  /////End of Wifi Things/////
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

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int contadorConexion = 0;
  while (WiFi.status() != WL_CONNECTED && contadorConexion<20) {
    delay(500);
    Serial.print(".");
    contadorConexion++;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == ubidotsTopicRele1)
  {
    Serial.print("Changing relay 1 output to ");
    if(messageTemp == "1")
    {
      Serial.println("on");
      miPlaca.relay(1,HIGH);
    }else if(messageTemp == "0")
    {
      Serial.println("off");
      miPlaca.relay(1,LOW);
    }
  }
  if (String(topic) == ubidotsTopicRele2)
  {
    Serial.print("Changing relay 2 output to ");
    if(messageTemp == "1")
    {
      Serial.println("on");
      miPlaca.relay(2,HIGH);
    }else if(messageTemp == "0")
    {
      Serial.println("off");
      miPlaca.relay(2,LOW);
    }
  }
  if (String(topic) == ubidotsTopicRele3 )
  {
    Serial.print("Changing relay 3 output to ");
    if(messageTemp == "1")
    {
      Serial.println("on");
      miPlaca.relay(3,HIGH);
    }else if(messageTemp == "0")
    {
      Serial.println("off");
      miPlaca.relay(3,LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientIDMQTT, userNameMQTT, passwordMQTT)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(ubidotsTopicRele1);
      client.subscribe(ubidotsTopicRele2);
      client.subscribe(ubidotsTopicRele3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
    }
  }
}

void loop()
{
  ////////////////////////////
  ////Get data from sensor////
  ////////////////////////////
  float temperatura, humedad;
  temperatura = miPlaca.getTemp();
  humedad = miPlaca.getHum();
  //Write to MQTT
  char payloadTempBuff[50];
  sprintf(payloadTempBuff, "%s %f}}", payloadTemp, temperatura);
  client.publish(ubidotsTopic, payloadTempBuff);
  char payloadHumBuff[50];
  sprintf(payloadHumBuff, "%s %f}}", payloadHum, humedad);
  client.publish(ubidotsTopic, payloadHumBuff);
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
  //Write to MQTT
  char payloadCurrentBuff[50];
  sprintf(payloadCurrentBuff, "%s %f}}", payloadCurr, corriente);
  client.publish(ubidotsTopic, payloadCurrentBuff);
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
    
    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);  
    wpsInitConfig();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
  }

  Serial.println("___________________________");Serial.println("");  
  delay(700);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
