#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DFRobot_AS7341.h>
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

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
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
    Visible light sensor
  ###########################*/
DFRobot_AS7341 as7341;
bool lightSensor = false;

void ConfigureSensors(void) {
//  //Integration time = (ATIME + 1) x (ASTEP + 1) x 2.78µs
//  //Set the value of register ATIME(1-255), through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
//  as7341.setAtime(29);
//  //Set the value of register ASTEP(0-65534), through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
//  as7341.setAstep(599);
//  //Set gain value(0~10 corresponds to X0.5,X1,X2,X4,X8,X16,X32,X64,X128,X256,X512)
//  as7341.setAGAIN(7);
//  Enable LED
//  as7341.enableLed(true);
//  Set pin current to control brightness (1~20 corresponds to current 4mA,6mA,8mA,10mA,12mA,......,42mA)
  as7341.controlLed(4);
}

void SearchSensors(void) {
  Serial.println("Searching/Checking connected I2C light sensors...");
  if (as7341.begin() == 0) {
    Serial.println("I2C Light Sensor found!");
    lightSensor = true;
    ConfigureSensors();
    Serial.println("Light Sensor configured!");
  } else {
    Serial.println("Please, connect an I2C Light Sensor..");
    lightSensor = false;
  }
};

void MeasureLight(uint16_t * spectrum) {
  DFRobot_AS7341::sModeOneData_t data1;
  DFRobot_AS7341::sModeTwoData_t data2;
  
  //Start spectrum measurement 
  //  Enable LED
  as7341.enableLed(true);
  delay(10);
  //Channel mapping mode: 1.eF1F4ClearNIR,2.eF5F8ClearNIR
  as7341.startMeasure(as7341.eF1F4ClearNIR);
  //Read the value of sensor data channel 0~5, under eF1F4ClearNIR
  data1 = as7341.readSpectralDataOne();
  spectrum[0] = data1.ADF1;
  spectrum[1] = data1.ADF2;
  spectrum[2] = data1.ADF3;
  spectrum[3] = data1.ADF4;
  Serial.print("F1(405-425nm):");
  Serial.println(data1.ADF1);
  Serial.print("F2(435-455nm):");
  Serial.println(data1.ADF2);
  Serial.print("F3(470-490nm):");
  Serial.println(data1.ADF3);
  Serial.print("F4(505-525nm):");   
  Serial.println(data1.ADF4);
  //Serial.print("Clear:");
  //Serial.println(data1.ADCLEAR);
  //Serial.print("NIR:");
  //Serial.println(data1.ADNIR);
  as7341.startMeasure(as7341.eF5F8ClearNIR);
  //Read the value of sensor data channel 0~5, under eF5F8ClearNIR
  data2 = as7341.readSpectralDataTwo();
  //  Disable LED
  as7341.enableLed(false);
  spectrum[4] = data2.ADF5;
  spectrum[5] = data2.ADF6;
  spectrum[6] = data2.ADF7;
  spectrum[7] = data2.ADF8;
  spectrum[8] = data2.ADNIR;
//  spectrum[8] = data2.ADCLEAR;
  Serial.print("F5(545-565nm):");
  Serial.println(data2.ADF5);
  Serial.print("F6(580-600nm):");
  Serial.println(data2.ADF6);
  Serial.print("F7(620-640nm):");
  Serial.println(data2.ADF7);
  Serial.print("F8(670-690nm):");
  Serial.println(data2.ADF8);
  Serial.print("Clear:");
  Serial.println(data2.ADCLEAR);
  Serial.print("NIR:");
  Serial.println(data2.ADNIR);
}




void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial.println("\nWelcome to BLE Light Sensor");

  // Create the BLE Device
  BLEDevice::init("Proto-Light");

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

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  
  // -----------------------------------------------------------------
  // Scan for light sensor
  Wire.begin();
  SearchSensors();
  // -----------------------------------------------------------------

}

void loop() {
  uint16_t spectrum [9] = { };


  // BLE routines
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("Start advertising");
      oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
//      pTxCharacteristic->setValue("Welcome to BLE Light Sensor");
//      pTxCharacteristic->notify();
      oldDeviceConnected = deviceConnected;
  }


  // Light Sensor routines
  SearchSensors();
  if (lightSensor) {
    MeasureLight(spectrum);
    Serial.println("**********");
    std::stringstream stream;
    std::string payload;
    for (int i = 0; i < sizeof(spectrum)/sizeof(spectrum[0]); i++) {
      stream << std::fixed << std::setprecision(2) << spectrum[i];
      if (i < (sizeof(spectrum)/sizeof(spectrum[0])-1)) {
        stream << "/";
      }
//      std::string s = to_string(spectrum[i]);
//      payload.append(s);
    }
    std::string s = stream.str();
//      payload.append(s);      
//    std::string sbegin = "\n";
//    sbegin.append(s);
//    sbegin.append(" °C");
    pTxCharacteristic->setValue(s);
    pTxCharacteristic->notify();
  }

  delay(5000);
}
