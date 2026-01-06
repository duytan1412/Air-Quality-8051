/*
 * ADS1115 16-BIT ADC DRIVER
 * Giao tiếp I2C, địa chỉ 0x48
 * 4 kênh single-ended, 16-bit, ±6.144V
 */

#include "ads1115.h"
#include "i2c.h"
#include "delay.h"

/*
 * Đọc 1 kênh ADC (chế độ single-shot)
 * Mỗi lần gọi sẽ cấu hình lại kênh và đọc
 */
int ADS1115_ReadChannel(unsigned char channel)
{
    unsigned char config_high, config_low;
    int result;

    // Config: OS=1 (start), MUX=channel, PGA=±6.144V, MODE=single-shot
    config_high = 0xC0 | (channel << 4) | 0x02;
    // DR=128SPS, Comparator disabled
    config_low = 0x83;

    // Ghi Config Register
    I2C_Start();
    I2C_WriteByte(ADS1115_ADDR << 1);
    I2C_WriteByte(0x01);
    I2C_WriteByte(config_high);
    I2C_WriteByte(config_low);
    I2C_Stop();

    // Chờ ADC chuyển đổi (~8ms với 128SPS)
    delay_ms(10);

    // Đọc Conversion Register
    I2C_Start();
    I2C_WriteByte(ADS1115_ADDR << 1);
    I2C_WriteByte(0x00);
    I2C_Stop();

    I2C_Start();
    I2C_WriteByte((ADS1115_ADDR << 1) | 0x01);
    result = (int)I2C_ReadByte(0) << 8;
    result |= I2C_ReadByte(1);
    I2C_Stop();

    return result;
}

/*
 * Bật chế độ continuous cho 1 kênh
 * ADC sẽ liên tục chuyển đổi với tốc độ 8 SPS
 */
void ADS1115_StartContinuous(unsigned char channel)
{
    unsigned char config_high, config_low;

    // Config: OS=1, MUX=channel, PGA=±6.144V, MODE=continuous
    config_high = 0xC0 | (channel << 4) | 0x02;
    // DR=8SPS (chậm để ổn định), Comparator disabled
    config_low = 0x03;

    I2C_Start();
    I2C_WriteByte(ADS1115_ADDR << 1);
    I2C_WriteByte(0x01);
    I2C_WriteByte(config_high);
    I2C_WriteByte(config_low);
    I2C_Stop();

    delay_ms(10);
}

/*
 * Đọc ADC trong chế độ continuous
 * Chỉ đọc Conversion Register, không cấu hình lại
 */
int ADS1115_ReadContinuous(void)
{
    int result;

    I2C_Start();
    I2C_WriteByte(ADS1115_ADDR << 1);
    I2C_WriteByte(0x00);
    I2C_Stop();

    I2C_Start();
    I2C_WriteByte((ADS1115_ADDR << 1) | 0x01);
    result = (int)I2C_ReadByte(0) << 8;
    result |= I2C_ReadByte(1);
    I2C_Stop();

    return result;
}
