#include <reg52.h>
#include "drivers/i2c.h"
#include "drivers/lcd.h"
#include "drivers/aht10.h"
#include "drivers/gp2y1014.h"
#include "drivers/mq2.h"
#include "drivers/delay.h"
#include "core/uart.h"
#include "core/uart_protocol.h"
#include "core/display.h"
#include "core/watchdog.h"

sbit KEY_MODE = P3^2;

int temperature_x10 = 0;
int humidity_x10 = 0;
unsigned int pm25_ugm3 = 0;
unsigned int pm25_raw = 0;
unsigned int gas_raw = 0;
unsigned char display_mode = 0;
volatile unsigned char key_pressed = 0;
unsigned int gas_ppm = 0;
unsigned char err_aht10 = 0;
unsigned char err_pm25 = 0;
unsigned char err_gas = 0;

// Bộ lọc trung bình cho Gas (loại bỏ spike nhiễu)
#define GAS_FILTER_SIZE 4
unsigned int gas_history[GAS_FILTER_SIZE] = {0, 0, 0, 0};
unsigned char gas_index = 0;

void INT0_ISR(void) interrupt 0
{
    key_pressed = 1;
}

#define AHT10_TEMP_MIN -400
#define AHT10_TEMP_MAX 850
#define AHT10_HUMID_MIN 0
#define AHT10_HUMID_MAX 1000
#define PM25_MAX 500
#define GAS_RAW_MIN 100
#define GAS_RAW_MAX 32000
#define MQ2_ZERO_THRESHOLD 150

// Hàm lọc trung bình - loại bỏ giá trị spike
unsigned int Gas_Filter(unsigned int new_value)
{
    unsigned long sum = 0;
    unsigned char i;
    unsigned int max_val = 0, min_val = 65535;
    
    // Lưu giá trị mới vào mảng
    gas_history[gas_index] = new_value;
    gas_index = (gas_index + 1) % GAS_FILTER_SIZE;
    
    // Tìm max và min
    for (i = 0; i < GAS_FILTER_SIZE; i++) {
        if (gas_history[i] > max_val) max_val = gas_history[i];
        if (gas_history[i] < min_val) min_val = gas_history[i];
        sum += gas_history[i];
    }
    
    // Loại bỏ max và min, tính trung bình 2 giá trị còn lại
    // Đây là "Trimmed Mean" - loại bỏ outlier
    sum = sum - max_val - min_val;
    return (unsigned int)(sum / (GAS_FILTER_SIZE - 2));
}

void main(void)
{
    I2C_Init();
    LCD_Init();
    UART_Init();
    AHT10_Init();
    GP2Y1014_Init();
    WDT_Init();
    
    IT0 = 1;
    EX0 = 1;
    EA = 1;

    LCD_SetCursor(0, 0);
    LCD_Print("Smart Room v1.0");
    delay_ms(2000);
    LCD_WriteCommand(0x01);
    delay_ms(2);

    while (1)
    {
        WDT_Feed();

        AHT10_Read(&temperature_x10, &humidity_x10);
        if (temperature_x10 < AHT10_TEMP_MIN || temperature_x10 > AHT10_TEMP_MAX ||
            humidity_x10 < AHT10_HUMID_MIN || humidity_x10 > AHT10_HUMID_MAX) {
            err_aht10 = 1;
        } else {
            err_aht10 = 0;
        }
        
        GP2Y1014_Read(&pm25_ugm3, &pm25_raw);
        if (pm25_raw == 0 || pm25_raw > 60000 || pm25_ugm3 > 500) {
            err_pm25 = 1;
        } else {
            err_pm25 = 0;
        }
        
        MQ2_Read(&gas_raw);
        if (gas_raw == 0 || gas_raw > 60000) {
            err_gas = 1;
        } else {
            err_gas = 0;
        }

        if (gas_raw > 32767) gas_raw = 32767;
        
        // Áp dụng bộ lọc trước khi tính ppm
        gas_raw = Gas_Filter(gas_raw);
        
        if (gas_raw > MQ2_ZERO_THRESHOLD)
        {
            gas_ppm = (gas_raw - MQ2_ZERO_THRESHOLD) / 33;
        }
        else
        {
            gas_ppm = 0;
        }
        
        if (gas_ppm > 1000) gas_ppm = 1000;

        if (key_pressed == 1)
        {
            delay_ms(20);
            if (KEY_MODE == 0)
            {
                display_mode++;
                if (display_mode > 3) display_mode = 0;
                LCD_WriteCommand(0x01);
                delay_ms(2);
            }
            key_pressed = 0;
        }

        Display_Update_Mode(temperature_x10, humidity_x10, pm25_ugm3, gas_ppm, display_mode,
                           err_aht10, err_pm25, err_gas);

        Send_DataToESP32_WithErrors(temperature_x10, humidity_x10, pm25_ugm3, gas_ppm,
                                    err_aht10, err_pm25, err_gas);

        delay_ms(500);
    }
}
