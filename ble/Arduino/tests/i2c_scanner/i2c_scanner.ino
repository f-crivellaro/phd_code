
#include <Wire.h>
#include "Protocentral_MAX30205.h"
MAX30205 tempSensor;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
  delay(100);
  
//  //init the hardware bmx160  
//  if (bmx160.begin() != true){
//    Serial.println("init false");
//    while(1);
//  }
//  delay(100);
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("Loop");
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second

  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
//    Serial.println(address);
//    Serial.println(error);
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(4000);                       // wait for a second  
}
