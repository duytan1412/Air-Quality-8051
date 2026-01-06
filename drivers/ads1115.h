/*
 * ============================================================================
 * ADS1115 ADC HEADER FILE - ads1115.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm điều khiển ADC ADS1115
 * Mục đích: Cung cấp interface để đọc giá trị analog từ cảm biến
 * ============================================================================
 */

#ifndef __ADS1115_H__
#define __ADS1115_H__

// Địa chỉ I2C của ADC ADS1115 (7-bit address)
#define ADS1115_ADDR 0x48

/*
 * Hàm: ADS1115_ReadChannel
 * Mô tả: Đọc giá trị ADC từ 1 kênh (chế độ single-shot)
 * Tham số: channel - Số kênh (0-3)
 * Trả về: Giá trị ADC 16-bit có dấu (-32768 đến 32767)
 * Lưu ý: Mất ~10ms để đọc (blocking)
 * Ví dụ: int adc = ADS1115_ReadChannel(0);  // Đọc kênh 0
 */
int ADS1115_ReadChannel(unsigned char channel);

/*
 * Hàm: ADS1115_StartContinuous
 * Mô tả: Bật chế độ đọc liên tục cho 1 kênh
 * Tham số: channel - Số kênh (0-3)
 * Trả về: Không
 * Lưu ý: Sau khi gọi, dùng ADS1115_ReadContinuous() để đọc
 * Ví dụ: ADS1115_StartContinuous(1);  // Bật continuous cho kênh 1
 */
void ADS1115_StartContinuous(unsigned char channel);

/*
 * Hàm: ADS1115_ReadContinuous
 * Mô tả: Đọc giá trị ADC trong chế độ continuous
 * Tham số: Không
 * Trả về: Giá trị ADC 16-bit có dấu (-32768 đến 32767)
 * Lưu ý: Phải gọi ADS1115_StartContinuous() trước
 * Ví dụ: int adc = ADS1115_ReadContinuous();
 */
int ADS1115_ReadContinuous(void);

#endif
