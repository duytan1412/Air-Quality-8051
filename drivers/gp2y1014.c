/*
 * GP2Y1014 DUST SENSOR DRIVER
 * Nguyên lý: LED hồng ngoại + Photodiode
 * Đọc qua ADC ADS1115 kênh 1
 */

#include "gp2y1014.h"
#include "ads1115.h"
#include "delay.h"

/*
 * CALIBRATION: Điện áp nền khi không có bụi (Zero Voltage)
 * Datasheet nói 500mV nhưng thực tế mỗi con khác nhau (600-900mV)
 * Cách đo: Đặt cảm biến trong không khí sạch, đọc voltage_mV
 * Giá trị mặc định 600mV phù hợp với đa số cảm biến
 */
#define GP2Y_ZERO_VOLTAGE 600

void GP2Y1014_Init(void)
{
    DUST_LED = 1;
}

void GP2Y1014_Read(unsigned int *pm25_ugm3, unsigned int *pm25_raw)
{
    long voltage_mV;
    int adc_signed;

    DUST_LED = 0;
    delay_us(280);

    adc_signed = ADS1115_ReadChannel(1);
    
    DUST_LED = 1;

    if (adc_signed < 0)
    {
        adc_signed = 0;
    }

    // Chuyển ADC sang Voltage (mV)
    voltage_mV = ((long)adc_signed * 6144L) / 32768L;

    // Chuyển Voltage sang PM2.5 với Zero Calibration
    // Chỉ tính khi voltage > ngưỡng nền
    if (voltage_mV > GP2Y_ZERO_VOLTAGE)
    {
        *pm25_ugm3 = (unsigned int)((voltage_mV - GP2Y_ZERO_VOLTAGE) / 6L);
    }
    else
    {
        *pm25_ugm3 = 0;
    }
    
    if (*pm25_ugm3 > 500) *pm25_ugm3 = 500;

    *pm25_raw = (unsigned int)adc_signed;
}
