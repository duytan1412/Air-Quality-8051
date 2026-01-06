/*
 * ============================================================================
 * UART PROTOCOL HEADER FILE - uart_protocol.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm xử lý giao thức UART
 * Mục đích: Cung cấp interface để gửi dữ liệu cảm biến qua UART
 * ============================================================================
 */

#ifndef __UART_PROTOCOL_H__
#define __UART_PROTOCOL_H__

/*
 * Hàm: Send_DataToESP32
 * Mô tả: Gửi dữ liệu cảm biến qua UART (không có thông tin lỗi)
 * Tham số:
 *   - temp_x10: Nhiệt độ nhân 10
 *   - humid_x10: Độ ẩm nhân 10
 *   - pm25: Nồng độ bụi PM2.5
 *   - gas: Nồng độ khí gas
 * Trả về: Không
 * Lưu ý: Hàm này hiện không được sử dụng trong dự án
 */
void Send_DataToESP32(int temp_x10, int humid_x10, unsigned int pm25, unsigned int gas);

/*
 * Hàm: Send_DataToESP32_WithErrors
 * Mô tả: Gửi dữ liệu cảm biến kèm thông tin lỗi qua UART
 * Tham số:
 *   - temp_x10: Nhiệt độ nhân 10 (ví dụ: 256 = 25.6°C)
 *   - humid_x10: Độ ẩm nhân 10 (ví dụ: 653 = 65.3%)
 *   - pm25: Nồng độ bụi PM2.5 (µg/m³)
 *   - gas: Nồng độ khí gas (ppm)
 *   - e1: Cờ lỗi cảm biến AHT10 (0=OK, 1=Error)
 *   - e2: Cờ lỗi cảm biến PM2.5 (0=OK, 1=Error)
 *   - e3: Cờ lỗi cảm biến Gas (0=OK, 1=Error)
 * Trả về: Không
 * Format JSON: {"T":25.6,"H":65.3,"P":35,"G":150,"E":0}\n
 * Lưu ý: Được gọi từ main.c mỗi 500ms
 */
void Send_DataToESP32_WithErrors(int temp_x10, int humid_x10, unsigned int pm25, 
                                  unsigned int gas, unsigned char e1, unsigned char e2, 
                                  unsigned char e3);

#endif
