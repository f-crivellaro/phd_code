/*
   Create a BLE server that, once we receive a connection, will send periodic temperature notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Wire.h>
#include <ProtoBeat.h>
#include <sstream>
#include <iomanip>
#include <string>

#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <freertos/task.h>
using namespace std;


/* ##########################
    Bluetooth Low Energy
  ###########################*/
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;
bool AFE_int = false;
#define MAX_SAMPLES 4096
uint32_t setSamples = 10;
time_t startTime;
TickType_t xStartTime, xSamplingTime;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // Service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println("");
        stringstream nSamples(rxValue);
        nSamples >> setSamples;
        if (setSamples > 0){
          AFE_int = true;
          time(&startTime);
          xStartTime = xTaskGetTickCount();
          Serial.println("Sampling starting");
          Serial.println(startTime);
          Serial.println(xStartTime);
        } else {
          AFE_int = false;
        }

        Serial.println();
        Serial.println("*********");
      }
    }
};

uint sample_cnt = 0;

ProtoBeat_Sensor beatSensor;

void IRAM_ATTR INThandler(void){
  // Do not use Serial.prints inside interruption handlers
    AFE_int = true;
    //sample_cnt++;
}


void systemStart(void){
    Serial.println("AFE Sensor Starting...");
    pinMode(RESET_STBY_GPIO, OUTPUT);
    pinMode(ADC_RDY_INT_GPIO, INPUT);
    beatSensor.reset();
    Wire.begin();

    if (beatSensor.scanAvailableSensors()){
        //attachInterrupt(ADC_RDY_INT_GPIO, INThandler, RISING);
        beatSensor.AFEconfig();
    }
}

void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial.println("\nWelcome to BLE Beat Sensor");

  // Create the BLE Device
  BLEDevice::init("Proto-Beat");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  /* ##########################
      Heart Rate 5 click board
    ###########################*/
  systemStart();

}

int32_t measurement = 0;
int32_t measurements[MAX_SAMPLES];
int32_t timestamps[MAX_SAMPLES];
#define INTERVAL_END_ARRAY  500
#define INTERVAL_SAMPLE     25
#define INTERVAL_BLE        50
int32_t timerEndArray = 0;
int32_t timerAuxEndArray = 0;
int32_t timerSampling = millis();
int32_t timerBLE = 0;


void loop() {

  if(millis() > timerSampling + INTERVAL_SAMPLE){
    if (AFE_int){
      Serial.println("Sampling...");
      measurement = beatSensor.getMeasurement();
      xSamplingTime = xTaskGetTickCount() - xStartTime;
      measurements[sample_cnt] = measurement;
      timestamps[sample_cnt] = xSamplingTime;
      sample_cnt++;

      if (sample_cnt > setSamples){
        Serial.println("Sampling End");
        AFE_int = false;
        sample_cnt = 0;
        timerSampling = 0;
        timerBLE = millis();
        if (deviceConnected) {
          Serial.println("Sending buffer through BLE...");
        }
      } else {
        timerSampling = millis();
      }
    }
  }

  if(millis() > timerBLE + INTERVAL_BLE){
    if (deviceConnected) {
      if (timerSampling == 0 and timerEndArray == 0){
        if (sample_cnt == setSamples){
          pTxCharacteristic->setValue("endArray");
          pTxCharacteristic->notify();
          Serial.println("Buffer sent");
          sample_cnt = 0;
          timerBLE = 0;
//          AFE_int = true;
//          timerSampling = millis();
          timerEndArray = millis();
          timerAuxEndArray = millis();
        } else {
          Serial.println("Sending data...");
          std::stringstream stream;
          stream << std::fixed << std::setprecision(2) << measurements[sample_cnt];
          stream << "/";
          stream << std::fixed << std::setprecision(2) << timestamps[sample_cnt];
          std::string s = stream.str();
          pTxCharacteristic->setValue(s);
          pTxCharacteristic->notify();
          sample_cnt++;
          timerBLE = millis();
        }
      }
    }
  }

  if(millis() < timerEndArray + INTERVAL_END_ARRAY){
    timerAuxEndArray = millis();
  } else {
    if (timerAuxEndArray > 0){
      timerEndArray = 0;
      timerAuxEndArray = 0;
      AFE_int = true;
      timerSampling = millis();
    }
  }


  // BLE routines
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    pTxCharacteristic->setValue("Welcome to BLE Beat Sensor");
    pTxCharacteristic->notify();
    oldDeviceConnected = deviceConnected;
  }
}
