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

#include <WiFi.h>
#include <PubSubClient.h>

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


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // Service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define MAX_SAMPLES             1024
#define INTERVAL_END_ARRAY      500
#define INTERVAL_SAMPLE         15
#define INTERVAL_BLE            10

#define WIFI_COM

#ifdef WIFI_COM
  const char* ssid = "Smart Devices";
  const char* password = "Compta202004";
#endif

bool AFE_ReadEnable = false;
int32_t measurement = 0;
int32_t measurements[MAX_SAMPLES];
int32_t redMeasurement = 0;
int32_t redMeasurements[MAX_SAMPLES];
int32_t amb1Measurement = 0;
int32_t amb1Measurements[MAX_SAMPLES];
int32_t amb2Measurement = 0;
int32_t amb2Measurements[MAX_SAMPLES];
int32_t BLEBufferMeasurements[MAX_SAMPLES];
int32_t BLEBufferRedMeasurements[MAX_SAMPLES];
int32_t BLEBufferAmb1Measurements[MAX_SAMPLES];
int32_t BLEBufferAmb2Measurements[MAX_SAMPLES];
uint32_t BLEBufferTimestamps[MAX_SAMPLES];

uint32_t setSamples = 100;           // Set of samples that are buffered (configurable)
uint32_t sample_cnt = 0;            // Counter for measurement readings
uint32_t data_cnt = 0;              // Counter for BLE data sending
uint32_t timestamps[MAX_SAMPLES];
uint32_t timerEndArray = 0;
uint32_t timerAuxEndArray = 0;
uint32_t timerSampling = 0xFFFFFFFF - INTERVAL_SAMPLE; // millis();
uint32_t timerBLE = 0xFFFFFFFF - INTERVAL_BLE; // 0;

ProtoBeat_Sensor beatSensor;

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
time_t startTime;
TickType_t xStartTime, xSamplingTime;

void IRAM_ATTR INThandler(void){
  // Do not use Serial.prints inside interruption handlers
    AFE_ReadEnable = true;
    //sample_cnt++;
}

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
      Serial.println(rxValue.length());
  
      if (rxValue.length() > 0) {
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println("");
        stringstream nSamples(rxValue);
        nSamples >> setSamples;
        Serial.print("Sampling starting with ");
        Serial.print(setSamples);
        Serial.println(" samples");
        if (setSamples > 0){
           detachInterrupt(ADC_RDY_INT_GPIO);
          AFE_ReadEnable = false;   // Stops to read measurements until next trigger
          beatSensor.reset();
          beatSensor.AFEconfig();
          time(&startTime);
          xStartTime = xTaskGetTickCount();
          Serial.print("Sampling starting with ");
          Serial.print(setSamples);
          Serial.println(" samples");
          Serial.println(startTime);
          Serial.println(xStartTime);
          attachInterrupt(ADC_RDY_INT_GPIO, INThandler, RISING);
//          timerSampling = millis();     // Restart the measurement reading
          sample_cnt = 0;
          data_cnt = 0;
          timerBLE = 0xFFFFFFFF - INTERVAL_BLE;   // Stop data transfer
        } else {
//          AFE_ReadEnable = false;
        }

        Serial.println();
        Serial.println("*********");
      }
    }
};



void systemStart(void){
    Wire.begin();

    if (beatSensor.scanAvailableSensors()){
        Serial.println("Heart Rate Sensor found!");
        pinMode(RESET_STBY_GPIO, OUTPUT);
        pinMode(ADC_RDY_INT_GPIO, INPUT);
        beatSensor.reset();
        beatSensor.powerdown();
//        beatSensor.reset();
//        attachInterrupt(ADC_RDY_INT_GPIO, INThandler, RISING);
//        beatSensor.AFEconfig();
    }
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("\nWelcome to BLE Beat Sensor");

  /* ##########################
      Bluetooth Low Energy
    ###########################*/
  BLEDevice::init("Proto-Beat");                    // Create the BLE Device
  pServer = BLEDevice::createServer();              // Create the BLE Server
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);  // Create the BLE Service

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



void loop() {
  
  // Measurement reading
  if (AFE_ReadEnable){
      measurement = beatSensor.getMeasurement(GREEN_LED); // Read the measurement data
      measurements[sample_cnt] = measurement;
      redMeasurement = beatSensor.getMeasurement(RED_LED);// Read the measurement data
      redMeasurements[sample_cnt] = redMeasurement;
      amb1Measurement = beatSensor.getMeasurement(AMBIENT_1);// Read the measurement data
      amb1Measurements[sample_cnt] = amb1Measurement;
      amb2Measurement = beatSensor.getMeasurement(AMBIENT_2);// Read the measurement data
      amb2Measurements[sample_cnt] = amb2Measurement;
      xSamplingTime = xTaskGetTickCount() - xStartTime;   // Establish the measurement timestamp
//      Serial.println(xSamplingTime);
      timestamps[sample_cnt] = xSamplingTime;
      AFE_ReadEnable = false;   // Stops to read measurements until next trigger
      sample_cnt++;
      if (sample_cnt > setSamples){
        Serial.println("All measurements were read");
        sample_cnt = 0;
        detachInterrupt(ADC_RDY_INT_GPIO);
        AFE_ReadEnable = false;   // Stops to read measurements until next trigger
        beatSensor.powerdown();
//        beatSensor.reset();
        Serial.println("Sensor in power down mode");
        if (deviceConnected) {
          Serial.println("Writing buffers..");
          for (uint32_t i = 0; i < setSamples; i++) {
            BLEBufferMeasurements[i] = measurements[i];
            BLEBufferRedMeasurements[i] = redMeasurements[i];
            BLEBufferAmb1Measurements[i] = amb1Measurements[i];
            BLEBufferAmb2Measurements[i] = amb2Measurements[i];
            BLEBufferTimestamps[i] = timestamps[i];
          }
          timerBLE = 0;           // Enable the BLE transmission
          Serial.println("Starting to send buffer through BLE...");
        }
      }
  }

  // Timer to not overload the BLE communication
  if (deviceConnected) {
    if(millis() > timerBLE + INTERVAL_BLE){
      if (data_cnt == setSamples){
        pTxCharacteristic->setValue("endArray");
        pTxCharacteristic->notify();
        Serial.println("BLE transfer ended");
        data_cnt = 0;
        timerBLE = 0xFFFFFFFF - INTERVAL_BLE;   // Stop data transfer
        beatSensor.reset();
        beatSensor.AFEconfig();
        time(&startTime);
        xStartTime = xTaskGetTickCount();
        Serial.print("Sampling starting with ");
        Serial.print(setSamples);
        Serial.println(" samples");
//        Serial.println(startTime);
//        Serial.println(xStartTime);
        attachInterrupt(ADC_RDY_INT_GPIO, INThandler, RISING);
      } else {
//        Serial.println("Sending data...");
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << BLEBufferMeasurements[data_cnt];
        stream << "/";
        stream << std::fixed << std::setprecision(2) << BLEBufferTimestamps[data_cnt];
        stream << "/";
        stream << std::fixed << std::setprecision(2) << BLEBufferRedMeasurements[data_cnt];
        stream << "/";
        stream << std::fixed << std::setprecision(2) << BLEBufferAmb1Measurements[data_cnt];
        stream << "/";
        stream << std::fixed << std::setprecision(2) << BLEBufferAmb2Measurements[data_cnt];
        std::string s = stream.str();
        pTxCharacteristic->setValue(s);
        pTxCharacteristic->notify();
//        Serial.print("Size of transfer: ");
//        Serial.println(sizeof(s));
        data_cnt++;
        timerBLE = millis();
      }
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
