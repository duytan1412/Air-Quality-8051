#ifndef __I2C_H__
#define __I2C_H__

#include <reg52.h>

sbit SDA = P2 ^ 2;
sbit SCL = P2 ^ 1;

#define I2C_TIMEOUT 200

extern unsigned char i2c_error;

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
bit I2C_WriteByte(unsigned char dat);
unsigned char I2C_ReadByte(bit ack);

#endif
