#include "EspMQTTClient.h"
#include <string.h>
#include "defines.h"
#include <SPI.h>

static const int spiClk = 1000000; // 1 MHz

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;
SPIClass * hspi = NULL;

EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASSWORD,
  "192.168.1.4",  // MQTT Broker server ip
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void SPI_Init(void)
{
  //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  vspi = new SPIClass(VSPI);
  //hspi = new SPIClass(HSPI);

  //initialise vspi with default pins
  //SCLK = 18, MISO = 19, MOSI = 23, SS = 5
  vspi->begin();

  //initialise hspi with default pins
  //SCLK = 14, MISO = 12, MOSI = 13, SS = 15
  //hspi->begin();

  //set up slave select pins as outputs as the Arduino API
  //doesn't handle automatically pulling SS low
  pinMode(vspi->pinSS(), OUTPUT); //VSPI SS
  //pinMode(hspi->pinSS(), OUTPUT); //HSPI SS
}

void SPI_Write(word data)
{
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(vspi->pinSS(), LOW); // begin the transfer
  
  byte low_byte = data & 0x00FF;
  byte high_byte = (data >> 8) & 0x00FF;
  
  vspi->transfer(low_byte);
  vspi->transfer(high_byte);
  
  digitalWrite(vspi->pinSS(), HIGH); // end the transfer
  vspi->endTransaction();
}

void setup()
{
  pinMode(17, OUTPUT); // Reset
  Serial.begin(115200);

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  //client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

  static const int spiClk = 1000000; // 1 MHz
  //uninitalised pointers to SPI objects
  SPIClass * vspi = NULL;
  SPIClass * hspi = NULL;
  

  Serial.println("Resetting DDS..");
  digitalWrite(17, LOW);
  delay(500);
  digitalWrite(17, HIGH);
  delay(500);
  digitalWrite(17, LOW);
  Serial.println("SPI Beginning..");
  SPI_Init();
  SPI_Write(0x1040);
  Serial.println("Setting Frequency..");
  SPI_Write(0x54E6);
  //SPI_Write(0x4A73);
  Serial.println("All Setup Done!");
  
}

//void setTxGain(int gain) {
//  Serial.println("TX Gain Setting of: %i", gain);
//}
//
//void setRxGain(int gain) {
//  Serial.println("RX Gain Setting of: %i", gain);
//}

void MQTTSubCallback(const String topic, const String payload) {
    char tmp[50];
    payload.toCharArray(tmp, payload.length());

    char *token = strtok(tmp, "-");

    if (!strncmp("tx", token, 2)) {
      Serial.println("Received a TX Gain configuration");
      token = strtok(NULL, "-");
      //setTxGain(atoi(token));
    } else if (!strncmp("rx", token, 2)) {
      Serial.println("Received a RX Gain configuration");
      token = strtok(NULL, "-");
      Serial.println(atoi(token));
      //setRxGain(atoi(token));      
    }
    client.publish("hunter/reply", "Done!!");
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to topic and display received message to Serial
  //client.subscribe("hunter/command", [](const String & topic, const String & payload) {
  //client.subscribe("hunter/command", [](const String & topic, const String & payload) {
  client.subscribe("hunter/command", &MQTTSubCallback);

  // Publish a message to "mytopic/test"
  //client.publish("hunter/sts", "This is a message"); // You can activate the retain flag by setting the third parameter to true
  client.publish("hunter/sts", "1");
}

void loop()
{
  client.loop();
}
