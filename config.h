#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <reg52.h>

// LCD 16x2 Pins (8-bit mode)
#define LCD_DATA P0
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;

// Buttons

sbit BTN_K1 = P3^0;
sbit BTN_K2 = P3^1;
sbit BTN_K3 = P3^2;
sbit BTN_K4 = P3^3;
#define BTN_MODE BTN_K3

// I2C Pins (Software Bit-bang)
sbit SDA = P1^0;
sbit SCL = P1^1;

// GP2Y1014 Dust Sensor Pins

sbit DUST_LED = P1^2;

// Buzzer
sbit BUZZER = P2^3;

// I2C Addresses

#define AHT10_ADDR   0x38
#define ADS1115_ADDR 0x48

// Air Quality Thresholds

#define PM25_GOOD      25
#define PM25_MODERATE  50
#define PM25_POOR      70
#define PM25_BAD       150

// Gas Threshold (ppm)
#define GAS_THRESHOLD  300

#endif
