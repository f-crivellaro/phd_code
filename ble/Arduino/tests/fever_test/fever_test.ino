
#include <Wire.h>
#include "Protocentral_MAX30205.h"
MAX30205 tempSensor;

// the setup function runs once when you press reset or power the board
void setup() {
  
  delay(1000);
  Wire.begin();
  Serial.begin(115200);
  delay(100);
  Serial.println("\nTemperature Sensor Firmware");
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //scan for temperature in every 30 sec untill a sensor is found. Scan for both addresses 0x48 and 0x49
  while(!tempSensor.scanAvailableSensors()){
    Serial.println("Couldn't find the temperature sensor, please connect the sensor." );
    delay(5000);
  }
  
  tempSensor.begin();   // set continuos mode, active mode
  Serial.println("\nSensor configured");
}

// the loop function runs over and over again forever
void loop() {

  float temp = tempSensor.getTemperature(); // read temperature for every 100ms
  Serial.print(temp ,2);
  Serial.println("'c" );
  delay(1000); 

}
