#ifndef TYPES_H
#define TYPES_H

struct SensorData {
    char temperature;
    unsigned char humidity;
    unsigned char temp_hum_valid;

    unsigned int light_lux;
    unsigned char light_valid;

    unsigned int lpg_ppm;
    unsigned int co_ppm;
    unsigned int smoke_ppm;
    unsigned char gas_valid;

    unsigned int pm25_ugm3;
    unsigned char dust_valid;

    unsigned char system_on;
    unsigned char error_flags;
};

#define ERROR_AHT10_NO_RESPONSE   0x01
#define ERROR_AHT10_CHECKSUM      0x02
#define ERROR_I2C_BUS_FAULT       0x04
#define ERROR_ADS1115_NO_RESPONSE 0x08
#define ERROR_MQ2_INVALID_DATA    0x10
#define ERROR_GP2Y_INVALID_DATA   0x20

#endif
