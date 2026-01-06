/*
 * ============================================================================
 * DISPLAY HEADER FILE - display.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm xử lý hiển thị LCD
 * Mục đích: Cung cấp interface để hiển thị dữ liệu cảm biến lên LCD
 * ============================================================================
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/*
 * Hàm: Display_Update_Mode
 * Mô tả: Cập nhật hiển thị LCD theo chế độ được chọn
 * Tham số:
 *   - temp_x10: Nhiệt độ nhân 10 (ví dụ: 256 = 25.6°C)
 *   - humid_x10: Độ ẩm nhân 10 (ví dụ: 653 = 65.3%)
 *   - pm25: Nồng độ bụi PM2.5 (µg/m³)
 *   - gas: Nồng độ khí gas (ppm)
 *   - mode: Chế độ hiển thị (0=Temp/Humid, 1=PM2.5, 2=Gas, 3=Auto)
 *   - err_aht10: Cờ lỗi cảm biến AHT10 (0=OK, 1=Error)
 *   - err_pm25: Cờ lỗi cảm biến PM2.5 (0=OK, 1=Error)
 *   - err_gas: Cờ lỗi cảm biến Gas (0=OK, 1=Error)
 * Trả về: Không
 * Lưu ý:
 *   - Chế độ Auto tự động chuyển đổi mỗi 2 giây
 *   - Hiển thị "ERR" nếu có lỗi cảm biến
 */
void Display_Update_Mode(int temp_x10, int humid_x10, unsigned int pm25, unsigned int gas, 
                         unsigned char mode, unsigned char err_aht10, unsigned char err_pm25, 
                         unsigned char err_gas);

/*
 * Hàm: Display_SensorError
 * Mô tả: Hiển thị màn hình lỗi cảm biến tập trung
 * Tham số:
 *   - err_aht10: Cờ lỗi cảm biến AHT10 (0=OK, 1=Error)
 *   - err_pm25: Cờ lỗi cảm biến PM2.5 (0=OK, 1=Error)
 *   - err_gas: Cờ lỗi cảm biến Gas (0=OK, 1=Error)
 * Trả về: Không
 * Format: 
 *   Dòng 1: SENSOR ERROR!
 *   Dòng 2: T:O H:O P:X G:O (O=OK, X=Error)
 * Lưu ý: Hiện không được sử dụng trong main.c
 */
void Display_SensorError(unsigned char err_aht10, unsigned char err_pm25, unsigned char err_gas);

#endif
