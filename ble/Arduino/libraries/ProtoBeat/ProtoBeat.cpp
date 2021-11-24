#include "Arduino.h"
#include <Wire.h>
#include <cmath>
#include "ProtoBeat.h"


bool ProtoBeat_Sensor::scanAvailableSensors(void){
  bool sensorFound = false;

  Wire.beginTransmission(I2C_AFE_ADDRESS);
  if (Wire.endTransmission () == 0){
    sensorFound = true;
    Serial.println("AFE Sensor found!");
  } else {
    Serial.println("AFE Sensor not found!");
  }

  return sensorFound;
}


void ProtoBeat_Sensor::AFEwrite(uint8_t WRaddr, uint32_t WRdata){
    uint8_t buf[3];
    uint32_t rem1;

	Wire.beginTransmission(I2C_AFE_ADDRESS);    // Initialize the I2C Tx buffer

    Wire.write(WRaddr);

    buf[0] = (uint8_t) (WRdata / 0x10000);
    rem1 = (WRdata % 0x10000);
    buf[1] = (uint8_t) (rem1 / 0x100);
    buf[2] = (uint8_t) rem1 % 0x100;

    // Serial.println("Putting data in I2C TX Buffer:");
    for (uint8_t i=0; i<I2C_DATA_LENGTH; i++){
        Wire.write(buf[i]);                     // Put data in Tx buffer
        // Serial.print("Byte ");
        // Serial.print(i);
        // Serial.print(": 0x");
        // Serial.print(buf[i], HEX);
        // Serial.println("");
    }
	Wire.endTransmission();                     // Send the Tx buffer
}


uint32_t ProtoBeat_Sensor::AFEread(uint8_t RDaddr){
    uint8_t buf[3];
    uint32_t tmp = 0;

	Wire.beginTransmission(I2C_AFE_ADDRESS);    
	Wire.write(RDaddr);
	Wire.endTransmission(false);
	Wire.requestFrom((uint16_t) I2C_AFE_ADDRESS, (uint8_t) 3);
    for (uint8_t i=0; i<I2C_DATA_LENGTH; i++){
        buf[i] = Wire.read();
        // Serial.println(buf[i], HEX);
    }
    tmp = buf[0]*pow(2, 16) + buf[1]*pow(2,8) + buf[2];
	return tmp;
}


void ProtoBeat_Sensor::AFEconfig(void){
    Serial.println("Enabling Register write mode");
    AFEwrite(0x00, 0);
    Serial.println("Enable internal oscillator (4 MHz)");
    AFEwrite(0x23, 131584);
    // AFEwrite(0x23, 131586);
    Serial.println("Enable the readout of write registers");
    AFEwrite(0x00, 1);
    Serial.println("Read Addr 0x23 to check OSC_ENABLE (bit 9)");
    Serial.println(AFEread(0x23));
    Serial.println("AFE Final config started");
    AFEwrite(0x00, 0);
    AFEwrite(0x39, 0);
    AFEwrite(0x1D, 39999);
    AFEwrite(0x09, 0);
    AFEwrite(0x0A, 398);
    AFEwrite(0x01, 100);
    AFEwrite(0x02, 398);
    AFEwrite(0x15, 5600);
    AFEwrite(0x16, 5606);
    AFEwrite(0x0D, 5608);
    AFEwrite(0x0E, 6067);
    AFEwrite(0x36, 400);
    AFEwrite(0x37, 798);
    AFEwrite(0x05, 500);
    AFEwrite(0x06, 798);
    AFEwrite(0x17, 6069);
    AFEwrite(0x18, 6075);
    AFEwrite(0x0F, 6077);
    AFEwrite(0x10, 6536);
    AFEwrite(0x03, 800);
    AFEwrite(0x04, 1198);
    AFEwrite(0x07, 900);
    AFEwrite(0x08, 1198);
    AFEwrite(0x19, 6538);
    AFEwrite(0x1A, 6544);
    AFEwrite(0x11, 6546);
    AFEwrite(0x12, 7006);
    AFEwrite(0x0B, 1300);
    AFEwrite(0x0C, 1598);
    AFEwrite(0x1B, 7008);
    AFEwrite(0x1C, 7014);
    AFEwrite(0x13, 7016);
    AFEwrite(0x14, 7475);
    AFEwrite(0x32, 7675);
    AFEwrite(0x33, 39199);

    AFEwrite(0x1E, 258);
    AFEwrite(0x20, 32772);
    AFEwrite(0x21, 3);
    AFEwrite(0x22, 12495);

    Serial.println("Enable the readout of write registers");
    AFEwrite(0x00, 1);
    Serial.println("AFE Final config ended");
}


void ProtoBeat_Sensor::reset(void){
    digitalWrite(RESET_STBY_GPIO, 1);
    digitalWrite(RESET_STBY_GPIO, 0);
    delay(0.025);
    digitalWrite(RESET_STBY_GPIO, 1);
    Serial.println("AFE Sensor resetting done!");

}


int32_t ProtoBeat_Sensor::getMeasurement(void){
    uint32_t data = 0;
    data = AFEread(0x2F);
    if (data > 0x1FFFFF){
        return (int32_t) (-1) * (0xFFFFFF - data + 1);
    } else {
        return (int32_t) data;
    }

}

