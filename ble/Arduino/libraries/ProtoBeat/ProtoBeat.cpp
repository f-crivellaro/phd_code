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
    AFEwrite(0x00, 0);          // Registers in write mode
    Serial.println("Enable internal oscillator (4 MHz)");
    AFEwrite(0x23, 131584);     
    // AFEwrite(0x23, 131586);
    Serial.println("Enable the readout of write registers");
    AFEwrite(0x00, 1);          // Registers in read mode
    Serial.println("Read Addr 0x23 to check OSC_ENABLE (bit 9)");
    Serial.println(AFEread(0x23));
    Serial.println("AFE Final config started");
    AFEwrite(0x00, 0);          // Registers in write mode
    // The configuration below is from Table 12 (pg. 28 of AFE4404 datasheet) for a 10 ms cycle with Three LEDs each with a duty cycle of 1 %, corresponding to a pulse duration of 100 us and four averages
    AFEwrite(0x39, 0);          // Definition of Pulse Repetition Frequency (PRF) Led On Freq 61 Hz minimum
    AFEwrite(0x1D, 39999);      // PRPCT PRF Counter for 100 Hz (CLK 4 MHz)
    AFEwrite(0x09, 0);          // LED2LEDSTC
    AFEwrite(0x0A, 398);        // LED2LEDENDC
    AFEwrite(0x01, 100);        // LED2STC
    AFEwrite(0x02, 398);        // LED2ENDC
    AFEwrite(0x15, 5600);       // ADCRSTSTCT0
    AFEwrite(0x16, 5606);       // ADCRSTENDCT0
    AFEwrite(0x0D, 5608);       // LED2CONVST
    AFEwrite(0x0E, 6067);       // LED2CONVEND
    AFEwrite(0x36, 400);        // LED3LEDSTC
    AFEwrite(0x37, 798);        // LED3LEDENDC
    AFEwrite(0x05, 500);        // ALED2STC / LED3STC
    AFEwrite(0x06, 798);        // ALED2ENDC / LED3ENDC
    AFEwrite(0x17, 6069);       // ADCRSTSTCT1
    AFEwrite(0x18, 6075);       // ADCRSTENDCT1
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

    AFEwrite(0x1E, 258);        // ADC averages = 4 
    AFEwrite(0x20, 32772);      // Configuration of the transimpedance amplifier
    AFEwrite(0x21, 2);          // Configuration of the transimpedance amplifier
    AFEwrite(0x22, 10425);     // 20 mA in all LEDs - 104025)

    Serial.println("Enable the readout of write registers");
    AFEwrite(0x00, 1);          // Registers in read mode
    Serial.println("AFE Final config ended");
}


void ProtoBeat_Sensor::reset(void){
    digitalWrite(RESET_STBY_GPIO, 1);
    digitalWrite(RESET_STBY_GPIO, 0);
    delay(0.025);
    digitalWrite(RESET_STBY_GPIO, 1);
    Serial.println("AFE Sensor resetting done!");

}


int32_t ProtoBeat_Sensor::getMeasurement(uint8_t selector){
    uint32_t led_data = 0;
    uint32_t ambient_data = 0;
    uint32_t data = 0;

    switch(selector) {
        case GREEN_LED: // LED1-AMBIENT
            data = AFEread(0x2F);
        break;
        case RED_LED:   // LED2VAL
            led_data = AFEread(0x2A);
            ambient_data = AFEread(0x2D);
            data = led_data - ambient_data;
        break;
        case IR_LED:    // LED3VAL
            data = AFEread(0x2B);
            ambient_data = AFEread(0x2D);
            data = led_data - ambient_data;
        break;
        case AMBIENT:   // ALED1VAL
            data = AFEread(0x2D);
        break;
        default:
            data = AFEread(0x2D);
    }

    if (data > 0x1FFFFF){
        return (int32_t) (-1) * (0xFFFFFF - data + 1);
    } else {
        return (int32_t) data;
    }
}



