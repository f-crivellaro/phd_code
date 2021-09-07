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
#include "Protocentral_MAX30205.h"
#include <sstream>
#include <iomanip>
#include <string>


/* ##########################
    Bluetooth Low Energy
  ###########################*/
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

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
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};


/* ##########################
    Fever click board
  ###########################*/
MAX30205 tempSensor;
bool feverSensor = false;

void SearchSensors(void) {
  Serial.println("Searching/Checking connected sensors...");
  if (tempSensor.scanAvailableSensors()) {
    Serial.println("I2C Fever Sensor found!");
    feverSensor = true;
    tempSensor.begin();   // set continuos mode, active mode
    Serial.println("Fever Sensor configured!");
  } else {
    Serial.println("Please, connect an I2C Fever Sensor..");
    feverSensor = false;
  }
};


void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial.println("\nWelcome to BLE Fever Sensor");

  // Create the BLE Device
  BLEDevice::init("Proto-Fever");

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

  // -----------------------------------------------------------------
  // Scan for temperature sensors untill a sensor is found. Scan for both addresses 0x48 and 0x49
  Wire.begin();
  SearchSensors();
  // -----------------------------------------------------------------
}

void loop() {

  // BLE routines
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("Start advertising");
      oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
      pTxCharacteristic->setValue("Welcome to BLE Fever Sensor (Celsius)");
      pTxCharacteristic->notify();
      oldDeviceConnected = deviceConnected;
  }

  // Fever Sensor routines
  SearchSensors();
  if (feverSensor) {
    float temp = tempSensor.getTemperature(); // read temperature for every 100ms
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << temp;
    std::string s = stream.str();
//    std::string sbegin = "\n";
//    sbegin.append(s);
//    sbegin.append(" °C");
    pTxCharacteristic->setValue(s);
    pTxCharacteristic->notify();
    Serial.print(temp ,2);
    Serial.println(" °C");
  }
  
  delay(5000);
}
