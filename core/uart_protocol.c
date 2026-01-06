/*
 * ============================================================================
 * UART PROTOCOL MODULE - uart_protocol.c
 * ============================================================================
 * Mô tả: Module xử lý giao thức truyền dữ liệu qua UART
 * Chức năng: Đóng gói dữ liệu cảm biến thành định dạng JSON và gửi đến ESP32
 * 
 * Định dạng JSON:
 * {"T":25.6,"H":65.3,"P":35,"G":150,"E":0}
 * - T: Temperature (nhiệt độ) - số thực 1 chữ số thập phân
 * - H: Humidity (độ ẩm) - số thực 1 chữ số thập phân
 * - P: PM2.5 (bụi mịn) - số nguyên
 * - G: Gas (khí gas) - số nguyên
 * - E: Error code (mã lỗi) - số nguyên (bit 0=AHT10, bit 1=PM25, bit 2=Gas)
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi main.c mỗi 500ms để gửi dữ liệu
 * - Sử dụng uart.c để gửi chuỗi JSON qua UART
 * - Sử dụng utils.c để chuyển đổi số thành chuỗi
 * - ESP32 nhận và parse JSON này để upload lên ThingsBoard
 * ============================================================================
 */

#include "uart_protocol.h"
#include "uart.h"
#include "utils.h"

/*
 * ============================================================================
 * HÀM: Send_DataToESP32_WithErrors
 * ============================================================================
 * Mô tả: Đóng gói dữ liệu cảm biến thành JSON và gửi qua UART đến ESP32
 * 
 * Tham số:
 * - temp_x10: Nhiệt độ nhân 10 (ví dụ: 256 = 25.6°C)
 * - humid_x10: Độ ẩm nhân 10 (ví dụ: 653 = 65.3%)
 * - pm25: Nồng độ bụi PM2.5 (µg/m³)
 * - gas: Nồng độ khí gas (ppm)
 * - e1: Cờ lỗi cảm biến AHT10 (0=OK, 1=Error)
 * - e2: Cờ lỗi cảm biến PM2.5 (0=OK, 1=Error)
 * - e3: Cờ lỗi cảm biến Gas (0=OK, 1=Error)
 * 
 * Chi tiết hoạt động:
 * 1. Tạo buffer 60 bytes trong XRAM để lưu chuỗi JSON
 * 2. Kết hợp 3 cờ lỗi thành 1 byte (bit masking)
 * 3. Xây dựng chuỗi JSON từng ký tự một (không dùng sprintf để tiết kiệm ROM)
 * 4. Gửi chuỗi JSON qua UART
 * 
 * Ví dụ output:
 * {"T":25.6,"H":65.3,"P":35,"G":150,"E":0}\n
 * 
 * Mối liên hệ:
 * - Được gọi từ main.c trong vòng lặp chính (mỗi 500ms)
 * - Gọi int_x10_to_str() và uint_to_str() từ utils.c
 * - Gọi UART_SendString() từ uart.c
 * - ESP32 nhận chuỗi này qua Serial1 (GPIO9)
 * ============================================================================
 */
void Send_DataToESP32_WithErrors(int temp_x10, int humid_x10, unsigned int pm25, 
                                  unsigned int gas, unsigned char e1, unsigned char e2, 
                                  unsigned char e3)
{
    // Khai báo buffer 60 bytes trong XRAM (external RAM)
    // Dùng XRAM vì IRAM (internal RAM) của 8051 chỉ có 128 bytes
    char xdata buffer[60];
    
    // Con trỏ p dùng để xây dựng chuỗi JSON từng ký tự
    char *p = buffer;
    
    // Biến lưu mã lỗi kết hợp (3 bit: AHT10, PM25, Gas)
    unsigned char err_combined;

    // ========================================================================
    // BƯỚC 1: KẾT HỢP CÁC CỜ LỖI THÀNH 1 BYTE
    // ========================================================================
    // Bit 0 (LSB): AHT10 error (e1)
    // Bit 1: PM2.5 error (e2)
    // Bit 2: Gas error (e3)
    // Ví dụ: e1=1, e2=0, e3=1 => err_combined = 0b101 = 5
    err_combined = (e1 ? 1 : 0) | (e2 ? 2 : 0) | (e3 ? 4 : 0);

    // ========================================================================
    // BƯỚC 2: XÂY DỰNG CHUỖI JSON TỪNG KÝ TỰ
    // ========================================================================
    // Lý do không dùng sprintf: tiết kiệm ROM (sprintf chiếm ~2KB)
    // Phương pháp: Ghi từng ký tự vào buffer, tăng con trỏ p
    
    // Bắt đầu JSON object
    *p++ = '{';
    
    // Thêm trường "T": (Temperature)
    *p++ = '"'; *p++ = 'T'; *p++ = '"'; *p++ = ':';
    // Chuyển temp_x10 thành chuỗi (ví dụ: 256 -> "25.6")
    p = int_x10_to_str(temp_x10, p);

    // Thêm trường "H": (Humidity)
    *p++ = ','; *p++ = '"'; *p++ = 'H'; *p++ = '"'; *p++ = ':';
    // Chuyển humid_x10 thành chuỗi (ví dụ: 653 -> "65.3")
    p = int_x10_to_str(humid_x10, p);

    // Thêm trường "P": (PM2.5)
    *p++ = ','; *p++ = '"'; *p++ = 'P'; *p++ = '"'; *p++ = ':';
    // Chuyển pm25 thành chuỗi (ví dụ: 35 -> "35")
    p = uint_to_str(pm25, p);

    // Thêm trường "G": (Gas)
    *p++ = ','; *p++ = '"'; *p++ = 'G'; *p++ = '"'; *p++ = ':';
    // Chuyển gas thành chuỗi (ví dụ: 150 -> "150")
    p = uint_to_str(gas, p);

    // Thêm trường "E": (Error code)
    *p++ = ','; *p++ = '"'; *p++ = 'E'; *p++ = '"'; *p++ = ':';
    // Chuyển err_combined thành ký tự số (0-7)
    *p++ = '0' + err_combined;

    // Kết thúc JSON object
    *p++ = '}';
    
    // Thêm ký tự xuống dòng (newline) - ESP32 dùng '\n' để phát hiện kết thúc gói
    *p++ = '\n';
    
    // Thêm null terminator để kết thúc chuỗi C
    *p = '\0';

    // ========================================================================
    // BƯỚC 3: GỬI CHUỖI JSON QUA UART
    // ========================================================================
    // Gửi toàn bộ buffer qua UART đến ESP32
    // ESP32 sẽ nhận và parse JSON này
    UART_SendString(buffer);
}
