#ifndef ProtoBeat_h
#define ProtoBeat_h

#include <stdint.h>

#define I2C_DATA_LENGTH    3    // Number of bytes expected in I2C data
#define I2C_AFE_ADDRESS    0x58 // AFE I2C address

#define RESET_STBY_GPIO    13//33
#define ADC_RDY_INT_GPIO   11//35

class ProtoBeat_Sensor {
    public:
        float   last_measurement = 0;             // Last sensor measurement
        // void    shutdown(void);                   // Instructs device to power-save
        void    reset(void);
        float   getMeasurement(void);
        bool    scanAvailableSensors(void);
        uint32_t    AFEread(uint8_t RDaddr);
        void        AFEconfig(void);

    private:
        void        AFEwrite(uint8_t WRaddr, uint32_t WRdata);
};

#endif