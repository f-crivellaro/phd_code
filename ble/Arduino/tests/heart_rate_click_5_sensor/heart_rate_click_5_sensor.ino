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


ProtoBeat_Sensor beatSensor;
bool AFE_int = false;

void IRAM_ATTR INThandler(void){
  // Do not use Serial.prints inside interruption handlers
    AFE_int = true;
}


void systemStart(void){
    Serial.println("AFE Sensor Starting...");
    pinMode(RESET_STBY_GPIO, OUTPUT);
    pinMode(ADC_RDY_INT_GPIO, INPUT_PULLUP);
    beatSensor.reset();
    Wire.begin();

    if (beatSensor.scanAvailableSensors()){
        attachInterrupt(ADC_RDY_INT_GPIO, INThandler, RISING);
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

void loop() {

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


  if (AFE_int){
    Serial.println(beatSensor.getMeasurement());
  }
  delay(1000);
}
