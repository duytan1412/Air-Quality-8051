/*
 * MQ2 GAS SENSOR DRIVER
 * Đọc qua ADC ADS1115 kênh 0
 * Phát hiện: LPG, Propane, Methane, CO, Smoke
 */

#include "mq2.h"
#include "ads1115.h"

/*
 * Đọc giá trị ADC từ cảm biến gas MQ2
 * Sử dụng Single-shot mode, kênh 0
 */
void MQ2_Read(unsigned int *gas_raw)
{
    int adc_signed;
    
    // Đọc kênh 0 (Single-shot mode)
    adc_signed = ADS1115_ReadChannel(0);
    
    // Xử lý giá trị âm từ ADC
    // ADS1115 có thể trả về giá trị âm khi nhiễu
    if (adc_signed < 0)
    {
        *gas_raw = 0;
    }
    else
    {
        *gas_raw = (unsigned int)adc_signed;
    }
}
