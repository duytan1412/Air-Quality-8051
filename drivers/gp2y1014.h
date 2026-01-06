/*
 * ============================================================================
 * GP2Y1014 DUST SENSOR HEADER FILE - gp2y1014.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm điều khiển cảm biến bụi GP2Y1014
 * Mục đích: Cung cấp interface để đọc nồng độ bụi PM2.5
 * ============================================================================
 */

#ifndef __GP2Y1014_H__
#define __GP2Y1014_H__

#include <reg52.h>

// Chân điều khiển LED hồng ngoại của cảm biến bụi
// Active low: 0=ON, 1=OFF
sbit DUST_LED = P2 ^ 0;

/*
 * Hàm: GP2Y1014_Init
 * Mô tả: Khởi tạo cảm biến bụi (tắt LED, bật ADC continuous)
 * Tham số: Không
 * Trả về: Không
 * Lưu ý: Phải gọi sau ADS1115_Init()
 */
void GP2Y1014_Init(void);

/*
 * Hàm: GP2Y1014_Read
 * Mô tả: Đọc nồng độ bụi PM2.5 từ cảm biến
 * Tham số:
 *   - pm25_ugm3: Con trỏ lưu nồng độ PM2.5 (µg/m³)
 *   - pm25_raw: Con trỏ lưu giá trị ADC thô
 * Trả về: Không
 * Lưu ý: Mất ~11ms để đọc (blocking)
 * Ví dụ:
 *   unsigned int pm25, raw;
 *   GP2Y1014_Read(&pm25, &raw);
 *   // pm25 = 35 (35 µg/m³)
 */
void GP2Y1014_Read(unsigned int *pm25_ugm3, unsigned int *pm25_raw);

#endif
