/*
 * ============================================================================
 * LCD 16x2 HEADER FILE - lcd.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm điều khiển LCD 16x2
 * Mục đích: Cung cấp interface để hiển thị dữ liệu lên LCD
 * ============================================================================
 */

#ifndef __LCD_H__
#define __LCD_H__

#include <reg52.h>

// Port dữ liệu LCD (8-bit mode)
// Kết nối: P0.0-P0.7 -> LCD D0-D7
#define LCD_DATA P0

// Chân điều khiển LCD
sbit LCD_RS = P2 ^ 6;  // Register Select: 0=Command, 1=Data
sbit LCD_RW = P2 ^ 5;  // Read/Write: 0=Write, 1=Read
sbit LCD_EN = P2 ^ 7;  // Enable: Xung để LCD đọc dữ liệu

/*
 * Hàm: LCD_Init
 * Mô tả: Khởi tạo LCD theo quy trình chuẩn HD44780
 * Tham số: Không
 * Trả về: Không
 * Lưu ý: Phải gọi trước khi sử dụng LCD
 */
void LCD_Init(void);

/*
 * Hàm: LCD_SetCursor
 * Mô tả: Di chuyển con trỏ đến vị trí (row, col)
 * Tham số:
 *   - row: Số dòng (0 hoặc 1)
 *   - col: Số cột (0-15)
 * Trả về: Không
 * Ví dụ: LCD_SetCursor(0, 5);  // Dòng 0, cột 5
 */
void LCD_SetCursor(unsigned char row, unsigned char col);

/*
 * Hàm: LCD_Print
 * Mô tả: Hiển thị chuỗi ký tự lên LCD
 * Tham số: str - Con trỏ đến chuỗi (kết thúc bằng '\0')
 * Trả về: Không
 * Lưu ý: Phải gọi LCD_SetCursor() trước
 * Ví dụ: LCD_Print("Hello");
 */
void LCD_Print(char *str);

/*
 * Hàm: LCD_WriteCommand
 * Mô tả: Gửi lệnh đến LCD
 * Tham số: cmd - Mã lệnh (0x01, 0x38, 0x0C, ...)
 * Trả về: Không
 * Ví dụ: LCD_WriteCommand(0x01);  // Clear Display
 */
void LCD_WriteCommand(unsigned char cmd);

/*
 * Hàm: LCD_WriteData
 * Mô tả: Gửi dữ liệu (ký tự) đến LCD
 * Tham số: dat - Mã ASCII của ký tự
 * Trả về: Không
 * Ví dụ: LCD_WriteData('A');  // Hiển thị 'A'
 */
void LCD_WriteData(unsigned char dat);

#endif
