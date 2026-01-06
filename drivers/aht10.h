/*
 * ============================================================================
 * AHT10 SENSOR HEADER FILE - aht10.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm điều khiển cảm biến AHT10
 * Mục đích: Cung cấp interface để đọc nhiệt độ và độ ẩm
 * ============================================================================
 */

#ifndef __AHT10_H__
#define __AHT10_H__

// Địa chỉ I2C của cảm biến AHT10 (7-bit address)
#define AHT10_ADDR 0x38

/*
 * Hàm: AHT10_Init
 * Mô tả: Khởi tạo cảm biến AHT10 (gửi lệnh calibration)
 * Tham số: Không
 * Trả về: Không
 * Lưu ý: Phải gọi trước khi đọc dữ liệu
 */
void AHT10_Init(void);

/*
 * Hàm: AHT10_Read
 * Mô tả: Đọc nhiệt độ và độ ẩm từ cảm biến
 * Tham số:
 *   - temp_x10: Con trỏ lưu nhiệt độ nhân 10 (ví dụ: 256 = 25.6°C)
 *   - humid_x10: Con trỏ lưu độ ẩm nhân 10 (ví dụ: 653 = 65.3%)
 * Trả về: Không
 * Lưu ý: Mất ~80ms để đọc (blocking)
 * Ví dụ:
 *   int temp, humid;
 *   AHT10_Read(&temp, &humid);
 *   // temp = 256 (25.6°C), humid = 653 (65.3%)
 */
void AHT10_Read(int *temp_x10, int *humid_x10);

#endif
