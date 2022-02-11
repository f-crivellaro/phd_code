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

    AFEwrite(0x09, 0);          // LED2LEDSTC - LED2 start
    AFEwrite(0x0A, 399);        // LED2LEDENDC - LED2 end
    AFEwrite(0x01, 100);        // LED2STC - Sample LED2 start
    AFEwrite(0x02, 399);        // LED2ENDC - Sample LED2 end
    AFEwrite(0x15, 401);       // ADCRSTSTCT0 - ADC reset phase 0 start
    AFEwrite(0x16, 407);       // ADCRSTENDCT0 - ADC reset phase 0 end
    AFEwrite(0x0D, 409);       // LED2CONVST - LED2 convert phase start
    AFEwrite(0x0E, 1468);       // LED2CONVEND - LED2 convert phase end

    AFEwrite(0x36, 401);        // LED3LEDSTC - LED3 start
    AFEwrite(0x37, 800);        // LED3LEDENDC - LED3 end
    AFEwrite(0x05, 501);        // ALED2STC/LED3STC- Sample ambient 2 (or sample LED3) start
    AFEwrite(0x06, 800);        // ALED2ENDC/LED3ENDC - Sample ambient 2 (or sample LED3) end
    AFEwrite(0x17, 1470);       // ADCRSTSTCT1 - ADC reset phase 1 start
    AFEwrite(0x18, 1476);       // ADCRSTENDCT1 - ADC reset phase 1 end
    AFEwrite(0x0F, 1478);       // ALED2CONVST/LED3CONVST - Ambient 2 (or LED3) convert phase start
    AFEwrite(0x10, 2537);       // ALED2CONVEND/LED3CONVEND - Ambient 2 (or LED3) convert phase end

    AFEwrite(0x03, 802);        // LED1LEDSTC - LED1 start
    AFEwrite(0x04, 1201);       // LED1LEDENDC - LED1 end
    AFEwrite(0x07, 902);        // LED1STC - Sample LED1 start
    AFEwrite(0x08, 1201);       // LED1ENDC - Sample LED1 end
    AFEwrite(0x19, 2539);       // ADCRSTSTCT2 - ADC reset phase 2 start
    AFEwrite(0x1A, 2545);       // ADCRSTENDCT2 - ADC reset phase 2 end
    AFEwrite(0x11, 2547);       // LED1CONVST - LED1 convert phase start
    AFEwrite(0x12, 3606);       // LED1CONVEND - LED1 convert phase end

    AFEwrite(0x0B, 1303);       // ALED1STC - Sample ambient 1 start
    AFEwrite(0x0C, 1602);       // ALED1ENDC - Sample ambient 1 end
    AFEwrite(0x1B, 3608);       // ADCRSTSTCT3 - ADC reset phase 3 start
    AFEwrite(0x1C, 3614);       // ADCRSTENDCT3 - ADC reset phase 3 end
    AFEwrite(0x13, 3616);       // ALED1CONVST - Ambient 1 convert phase start
    AFEwrite(0x14, 4675);       // ALED1CONVEND - Ambient 1 convert phase end

    // AFEwrite(0x09, 0);          // LED2LEDSTC - LED2 start
    // AFEwrite(0x0A, 399);        // LED2LEDENDC - LED2 end
    // AFEwrite(0x01, 100);        // LED2STC - Sample LED2 start
    // AFEwrite(0x02, 399);        // LED2ENDC - Sample LED2 end
    // AFEwrite(0x15, 5600);       // ADCRSTSTCT0 - ADC reset phase 0 start
    // AFEwrite(0x16, 5606);       // ADCRSTENDCT0 - ADC reset phase 0 end
    // AFEwrite(0x0D, 5608);       // LED2CONVST - LED2 convert phase start
    // AFEwrite(0x0E, 6067);       // LED2CONVEND - LED2 convert phase end
   

    // AFEwrite(0x36, 0);        // LED3LEDSTC - LED3 start
    // AFEwrite(0x37, 0);        // LED3LEDENDC - LED3 end
    // AFEwrite(0x05, 400);        // ALED2STC/LED3STC- Sample ambient 2 (or sample LED3) start
    // AFEwrite(0x06, 798);        // ALED2ENDC/LED3ENDC - Sample ambient 2 (or sample LED3) end
    // AFEwrite(0x17, 6069);       // ADCRSTSTCT1 - ADC reset phase 1 start
    // AFEwrite(0x18, 6075);       // ADCRSTENDCT1 - ADC reset phase 1 end
    // AFEwrite(0x0F, 6077);       // ALED2CONVST/LED3CONVST - Ambient 2 (or LED3) convert phase start
    // AFEwrite(0x10, 6536);       // ALED2CONVEND/LED3CONVEND - Ambient 2 (or LED3) convert phase end

    // AFEwrite(0x03, 800);        // LED1LEDSTC - LED1 start
    // AFEwrite(0x04, 1198);       // LED1LEDENDC - LED1 end
    // AFEwrite(0x07, 900);        // LED1STC - Sample LED1 start
    // AFEwrite(0x08, 1198);       // LED1ENDC - Sample LED1 end
    // AFEwrite(0x19, 6538);       // ADCRSTSTCT2 - ADC reset phase 2 start
    // AFEwrite(0x1A, 6544);       // ADCRSTENDCT2 - ADC reset phase 2 end
    // AFEwrite(0x11, 6546);       // LED1CONVST - LED1 convert phase start
    // AFEwrite(0x12, 7006);       // LED1CONVEND - LED1 convert phase end

    // AFEwrite(0x0B, 1200);       // ALED1STC - Sample ambient 1 start
    // AFEwrite(0x0C, 1598);       // ALED1ENDC - Sample ambient 1 end
    // AFEwrite(0x1B, 7008);       // ADCRSTSTCT3 - ADC reset phase 3 start
    // AFEwrite(0x1C, 7014);       // ADCRSTENDCT3 - ADC reset phase 3 end
    // AFEwrite(0x13, 7016);       // ALED1CONVST - Ambient 1 convert phase start
    // AFEwrite(0x14, 7475);       // ALED1CONVEND - Ambient 1 convert phase end

    // AFEwrite(0x32, 7675);       // PDNCYCLESTC
    AFEwrite(0x32, 5475);       // PDNCYCLESTC
    AFEwrite(0x33, 39199);      // PDNCYCLEENDC

    AFEwrite(0x1E, 259);        // ADC averages 
    AFEwrite(0x20, 32781);          // Configuration of the transimpedance amplifier
    // AFEwrite(0x20, 0);          // Configuration of the transimpedance amplifier
    AFEwrite(0x21, 12);          // Configuration of the transimpedance amplifier
    // AFEwrite(0x21, 13);          // Configuration of the transimpedance amplifier
    AFEwrite(0x22, 82708);      // Broadband photodiode
    // AFEwrite(0x22, 82713);      // Broadband photodiode
    // AFEwrite(0x22, 1922);      // IR-CUT photodiode
    AFEwrite(0x3A, 0);        // Offset cancelation trial 

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


void ProtoBeat_Sensor::powerdown(void){
    AFEwrite(0x23, 131585);
}


int32_t ProtoBeat_Sensor::getMeasurement(uint8_t selector){
    uint32_t led_data = 0;
    uint32_t ambient_data = 0;
    uint32_t data = 0;

    switch(selector) {
        case GREEN_LED: // LED1VAL
            data = AFEread(0x2C);
        break;
        case RED_LED:   // LED2VAL
            led_data = AFEread(0x2A);
            // ambient_data = AFEread(0x2D);
            data = led_data;// - ambient_data;
        break;
        case IR_LED:    // LED3VAL
            // data = AFEread(0x2B);
            //ambient_data = AFEread(0x2D);
            data = AFEread(0x2B);
        break;
        case AMBIENT_1:   // ALED1VAL
            data = AFEread(0x2D);
        break;
        case AMBIENT_2:   // ALED1VAL
            data = AFEread(0x2B);
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



