#ifndef ProtoBeat_h
#define ProtoBeat_h

#include <stdint.h>

#define I2C_DATA_LENGTH    3    // Number of bytes expected in I2C data
#define I2C_AFE_ADDRESS    0x58 // AFE I2C address

#define RESET_STBY_GPIO    33
#define ADC_RDY_INT_GPIO   35

#define GREEN_LED           1
#define RED_LED             2
#define IR_LED              3
#define AMBIENT_1           4
#define AMBIENT_2           5

class ProtoBeat_Sensor {
    public:
        int32_t last_measurement = 0;                // Last sensor measurement
        // void    shutdown(void);                   // Instructs device to power-save
        void        reset(void);
        int32_t     getMeasurement(uint8_t selector);
        bool        scanAvailableSensors(void);
        uint32_t    AFEread(uint8_t RDaddr);
        void        AFEconfig(void);

    private:
        void        AFEwrite(uint8_t WRaddr, uint32_t WRdata);
};

#endif