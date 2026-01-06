/*
 * ============================================================================
 * FILE CẤU HÌNH PHẦN CỨNG - CONFIG.H
 * ============================================================================
 * Mô tả: File định nghĩa tất cả các chân kết nối phần cứng và hằng số cấu hình
 * Mục đích: Tập trung tất cả cấu hình phần cứng ở một nơi, dễ bảo trì và sửa đổi
 * ============================================================================
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

// Thư viện chuẩn cho vi điều khiển 8051
#include <reg52.h>

// ============================================================================
// CẤU HÌNH CHÂN KẾT NỐI LCD 16x2
// ============================================================================

// Port dữ liệu LCD (8-bit mode) - kết nối với P0 (D0-D7)
// P0.0 -> LCD D0, P0.1 -> LCD D1, ..., P0.7 -> LCD D7
#define LCD_DATA P0

// Chân điều khiển LCD
sbit LCD_RS = P2^6;  // Register Select: 0=Command, 1=Data
sbit LCD_RW = P2^5;  // Read/Write: 0=Write, 1=Read (luôn dùng Write)
sbit LCD_EN = P2^7;  // Enable: tạo xung để LCD đọc dữ liệu

// ============================================================================
// CẤU HÌNH NÚT NHẤN
// ============================================================================

// Định nghĩa 4 nút nhấn kết nối với Port 3
sbit BTN_K1 = P3^0;  // Nút K1 - chân P3.0
sbit BTN_K2 = P3^1;  // Nút K2 - chân P3.1
sbit BTN_K3 = P3^2;  // Nút K3 - chân P3.2 (dùng làm nút chuyển chế độ)
sbit BTN_K4 = P3^3;  // Nút K4 - chân P3.3

// Định nghĩa nút chuyển chế độ hiển thị (sử dụng BTN_K3)
#define BTN_MODE BTN_K3

// ============================================================================
// CẤU HÌNH GIAO TIẾP I2C
// ============================================================================

// Chân SDA (Serial Data) - truyền dữ liệu I2C
sbit SDA = P1^0;

// Chân SCL (Serial Clock) - xung clock I2C
sbit SCL = P1^1;

// ============================================================================
// CẤU HÌNH CẢM BIẾN BỤI GP2Y1014
// ============================================================================

// Chân điều khiển LED hồng ngoại của cảm biến bụi
// LED phải được bật trong 0.28ms khi đọc ADC để đo chính xác
sbit DUST_LED = P1^2;

// ============================================================================
// CẤU HÌNH BUZZER (CHUÔNG BÁO)
// ============================================================================

// Chân điều khiển buzzer - dùng để cảnh báo khi có lỗi hoặc vượt ngưỡng
sbit BUZZER = P2^3;

// ============================================================================
// ĐỊA CHỈ I2C CỦA CÁC THIẾT BỊ
// ============================================================================

// Địa chỉ I2C của cảm biến nhiệt độ/độ ẩm AHT10 (7-bit address)
#define AHT10_ADDR   0x38

// Địa chỉ I2C của ADC ADS1115 (16-bit, 4 kênh)
// Dùng để đọc tín hiệu analog từ cảm biến bụi và gas
#define ADS1115_ADDR 0x48

// ============================================================================
// NGƯỠNG CHẤT LƯỢNG KHÔNG KHÍ PM2.5 (µg/m³)
// ============================================================================
// Theo tiêu chuẩn WHO và EPA về chất lượng không khí

#define PM25_GOOD      25   // Tốt: 0-25 µg/m³ (màu xanh lá)
#define PM25_MODERATE  50   // Trung bình: 26-50 µg/m³ (màu vàng)
#define PM25_POOR      70   // Kém: 51-70 µg/m³ (màu cam)
#define PM25_BAD       150  // Xấu: 71-150 µg/m³ (màu đỏ)
                            // Nguy hại: >150 µg/m³ (màu tím)

// ============================================================================
// NGƯỠNG CẢNH BÁO KHÍ GAS (ppm)
// ============================================================================

// Ngưỡng cảnh báo khí gas (LPG, CO, Smoke) - đơn vị ppm
// Vượt ngưỡng này cần cảnh báo người dùng
#define GAS_THRESHOLD  300

#endif
