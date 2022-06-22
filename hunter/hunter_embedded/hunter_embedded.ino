#include "EspMQTTClient.h"
#include <stdio.h>
#include <string.h>
#include "defines.h"
#include <SPI.h>

static const int spiClk = 400000; // 400 khz

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;
SPIClass * hspi = NULL;

EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASSWORD,
  "192.168.1.253",  // MQTT Broker server ip
  "CCCRiveLlAro",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

char frequency[20];

void SPI_Init(void)
{
  //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  vspi = new SPIClass(VSPI);
  hspi = new SPIClass(HSPI);

  //initialise vspi with default pins
  //SCLK = 18, MISO = 19, MOSI = 23, SS = 5
  vspi->begin();

  //initialise hspi with default pins
  //SCLK = 14, MISO = 12, MOSI = 13, SS = 15
  hspi->begin();

  //set up slave select pins as outputs as the Arduino API
  //doesn't handle automatically pulling SS low
  pinMode(vspi->pinSS(), OUTPUT); //VSPI SS
  pinMode(hspi->pinSS(), OUTPUT); //HSPI SS
}

void DDS_A_Write(word data) 
{
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE2));
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE2));
  digitalWrite(vspi->pinSS(), LOW); // begin the transfer
  
  byte low_byte = data & 0x00FF;
  byte high_byte = (data >> 8) & 0x00FF;
  
  vspi->transfer(high_byte);
  vspi->transfer(low_byte);
  
  hspi->transfer(high_byte);
  hspi->transfer(low_byte);
  
  digitalWrite(vspi->pinSS(), HIGH); // end the transfer
  vspi->endTransaction();
  hspi->endTransaction();
}

void DDS_B_Write(word data_a, word data_b) 
{
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE2));
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE2));
  digitalWrite(vspi->pinSS(), LOW); // begin the transfer

  byte low_byte = data_a & 0x00FF;
  byte high_byte = (data_a >> 8) & 0x00FF;

  vspi->transfer(high_byte);
  vspi->transfer(low_byte);

  low_byte = data_b & 0x00FF;
  high_byte = (data_b >> 8) & 0x00FF;
  
  hspi->transfer(high_byte);
  hspi->transfer(low_byte);
  
  digitalWrite(vspi->pinSS(), HIGH); // end the transfer
  vspi->endTransaction();
  hspi->endTransaction();
}

void ResetAll(void)
{
  DDS_A_Write(0x2100);

  delay(500);
  
  DDS_A_Write(0x2000);

  delay(500);
}




void setup()
{
  pinMode(RESET, OUTPUT);   // Reset
  pinMode(CLR, OUTPUT);     // Reset
  pinMode(DRV_EN, OUTPUT);  // DRV_EN
  pinMode(DRV_A0, OUTPUT);  // DRV_A0
  pinMode(DRV_A1, OUTPUT);  // DRV_A1
  pinMode(RX_EN, OUTPUT);   // RX_EN
  pinMode(RX_A0, OUTPUT);   // RX_A0
  pinMode(RX_A1, OUTPUT);   // RX_A1
  digitalWrite(RESET, LOW);  
  digitalWrite(CLR, HIGH);  
  digitalWrite(DRV_EN, LOW);  
  digitalWrite(DRV_A0, LOW);  
  digitalWrite(DRV_A1, LOW);  
  digitalWrite(RX_EN, LOW);  
  digitalWrite(RX_A0, LOW);  
  digitalWrite(RX_A1, LOW);  
  
  Serial.begin(115200);

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  //client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  //client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to tru
  
  Serial.println("Resetting All DDS..");
  SPI_Init();
  
  ResetAll();

//  digitalWrite(17, HIGH);
//  Serial.println("Reset High");
//  delay(1000);
//  digitalWrite(17, LOW);
//  Serial.println("Reset Low");
//  delay(2000);
  

//  Serial.println("Setting Phase..");
//  char phase[10];
//  sprintf(phase, "%s", 1024);
//  setPhase(phase);

  Serial.println("Setting Frequency..");
  sprintf(frequency, "%s", "500000");
  setFreq(frequency);
  //DDS_A_Write(0x54E6); // 1000 Hz
  //DDS_A_Write(0x4A7C); //500 Hz
  //DDS_A_Write(0x4000);
//  DDS_A_Write(30147); //500 kHz
//  DDS_A_Write(16547);

    //DDS_A_Write(29491); //10 MHz
    //DDS_A_Write(19660);
  
//  DDS_B_Write(30147); //500 kHz
//  DDS_B_Write(16547);
//  Serial.println("Resetting DDS..");
//  digitalWrite(17, HIGH);
//  delay(1000);
//  digitalWrite(17, LOW);
  Serial.println("All Setup Done!");
  
}

void setTxGain(char *gain) {
  char tmp[30];
  if (!strncmp("00", gain, 2)) {
    sprintf(tmp, "TX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(DRV_EN, LOW);  
  } else if (!strncmp("11", gain, 2)) {
    sprintf(tmp, "TX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(DRV_EN, HIGH);
    digitalWrite(DRV_A0, LOW);
    digitalWrite(DRV_A1, LOW);
  } else if (!strncmp("05", gain, 2)) {
    sprintf(tmp, "TX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(DRV_EN, HIGH);
    digitalWrite(DRV_A0, HIGH);
    digitalWrite(DRV_A1, LOW);
  } else if (!strncmp("03", gain, 2)) {
    sprintf(tmp, "TX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(DRV_EN, HIGH);
    digitalWrite(DRV_A0, LOW);
    digitalWrite(DRV_A1, HIGH);
  } else if (!strncmp("02", gain, 2)) {
    sprintf(tmp, "TX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(DRV_EN, HIGH);
    digitalWrite(DRV_A0, HIGH);
    digitalWrite(DRV_A1, HIGH);
  }
  
}

void setRxGain(char *gain) {
  char tmp[30];
  if (!strncmp("00", gain, 2)) {
    sprintf(tmp, "RX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(RX_EN, LOW);  
  } else if (!strncmp("11", gain, 2)) {
    sprintf(tmp, "RX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(RX_EN, HIGH);
    digitalWrite(RX_A0, LOW);
    digitalWrite(RX_A1, LOW);
  } else if (!strncmp("05", gain, 2)) {
    sprintf(tmp, "RX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(RX_EN, HIGH);
    digitalWrite(RX_A0, HIGH);
    digitalWrite(RX_A1, LOW);
  } else if (!strncmp("03", gain, 2)) {
    sprintf(tmp, "RX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(RX_EN, HIGH);
    digitalWrite(RX_A0, LOW);
    digitalWrite(RX_A1, HIGH);
  } else if (!strncmp("02", gain, 2)) {
    sprintf(tmp, "RX Gain Setting of: %s", gain);
    Serial.println(tmp);
    digitalWrite(RX_EN, HIGH);
    digitalWrite(RX_A0, HIGH);
    digitalWrite(RX_A1, HIGH);
  }
}


void setFreq(char *freqout) {
  char tmp[30];
  sprintf(tmp, "Frequency Setting of: %s", freqout);
  Serial.println(tmp);
  float freqout_float = atof(freqout);
  Serial.println(freqout_float);
  unsigned long freqreg = round(freqout_float/(DDS_MCLK/pow(2,28)));
  Serial.println(freqreg);
  Serial.println("Setting Frequency FREQ0");
  word freqreg_low = (freqreg & 0x3FFF) | 0x4000;
  word freqreg_high = (freqreg >> 14) | 0x4000;
  Serial.println(freqreg_low);
  Serial.println(freqreg_high);

//  ResetAll();

  Serial.println("Resetting");
  DDS_A_Write(0x2100);
  delay(500);
  
  Serial.println("Setting Frequency..");
  DDS_A_Write(freqreg_low);
  DDS_A_Write(freqreg_high);

    char phase[10];
  sprintf(phase, "%s", "2048");
  setPhase(phase);
  
  Serial.println("Ok");
}


void setPhase(char *phase) {
  char tmp[30];
  sprintf(tmp, "Phase Setting of: %s", phase);
  Serial.println(tmp);
  float phase_float = atof(phase);
  Serial.println(phase_float);
//  long int phasereg = round(phase_float*pow(2,12)/2*3.1415926);
  long int phasereg = round(phase_float);
  Serial.println(phasereg);
  Serial.println("Setting PHASE0 REG");
  word phasereg_low = (phasereg & 0x0FFF) | 0xC000;
  Serial.println(phasereg_low);

  Serial.println("Resetting");
  DDS_A_Write(0x2200);
  delay(500);
  digitalWrite(RESET, HIGH);
  Serial.println("Reset High");
  
//  DDS_A_Write(0x2300);
  delay(500);
  
  Serial.println("Writing Phase..");
  DDS_B_Write(0xC000, phasereg_low);


  digitalWrite(RESET, LOW);
  Serial.println("Reset Low");
  
//  DDS_A_Write(0x2000);
  delay(500);
  Serial.println("Ok");
}


void MQTTSubCallback(const String topic, const String payload) {
    char tmp[50];
    payload.toCharArray(tmp, payload.length());

    char *token = strtok(tmp, "-");

    if (!strncmp("tx", token, 2)) {
      Serial.println("Received a TX Gain configuration");
      token = strtok(NULL, "-");
      Serial.println(token);
      setTxGain(token);
    } else if (!strncmp("rx", token, 2)) {
      Serial.println("Received a RX Gain configuration");
      token = strtok(NULL, "-");
      Serial.println(token);
      setRxGain(token);      
    } else if (!strncmp("fr", token, 2)) {
      Serial.println("Received a Frequency configuration");
      token = strtok(NULL, "-");
      Serial.println(token);
      setFreq(token);
    } else if (!strncmp("ph", token, 2)) {
      Serial.println("Received a Phase configuration");
      token = strtok(NULL, "-");
      Serial.println(token);
      setPhase(token);
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
