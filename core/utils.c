/*
 * ============================================================================
 * UTILITY FUNCTIONS MODULE - utils.c
 * ============================================================================
 * Mô tả: Module chứa các hàm tiện ích chuyển đổi số thành chuỗi
 * Mục đích: Tránh dùng sprintf() để tiết kiệm ROM (sprintf chiếm ~2KB)
 * 
 * Lý do cần module này:
 * - 8051 không có FPU (Floating Point Unit)
 * - Thư viện sprintf() quá nặng cho 8051 (ROM hạn chế)
 * - Cần chuyển số thành chuỗi để hiển thị LCD và gửi JSON
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi uart_protocol.c để tạo chuỗi JSON
 * - Được gọi bởi display.c để hiển thị số lên LCD
 * ============================================================================
 */

#include "utils.h"

/*
 * ============================================================================
 * HÀM: uint_to_str
 * ============================================================================
 * Mô tả: Chuyển số nguyên không dấu (unsigned int) thành chuỗi
 * 
 * Tham số:
 * - val: Số nguyên cần chuyển (0 - 65535)
 * - buf: Con trỏ đến buffer để lưu chuỗi kết quả
 * 
 * Trả về: Con trỏ đến vị trí sau ký tự cuối cùng (trước '\0')
 * 
 * Ví dụ:
 * uint_to_str(0, buf)    -> "0"
 * uint_to_str(123, buf)  -> "123"
 * uint_to_str(5000, buf) -> "5000"
 * 
 * Thuật toán:
 * 1. Trường hợp đặc biệt: val = 0 -> ghi "0"
 * 2. Tách từng chữ số bằng phép chia lấy dư (val % 10)
 * 3. Lưu các chữ số vào mảng tạm (thứ tự ngược)
 * 4. Đảo ngược mảng tạm và ghi vào buffer
 * 
 * Mối liên hệ:
 * - Được gọi bởi uart_protocol.c để chuyển pm25, gas thành chuỗi
 * - Được gọi bởi display.c để hiển thị số lên LCD
 * ============================================================================
 */
char *uint_to_str(unsigned int val, char *buf)
{
    // Mảng tạm lưu các chữ số (tối đa 5 chữ số cho 65535)
    char temp[6];
    unsigned char i = 0;

    // Trường hợp đặc biệt: val = 0
    if (val == 0)
    {
        *buf++ = '0';  // Ghi ký tự '0' vào buffer
    }
    else
    {
        // Vòng lặp tách từng chữ số (từ phải sang trái)
        while (val > 0)
        {
            // Lấy chữ số cuối cùng (val % 10)
            // Chuyển thành ký tự ASCII bằng cách cộng với '0' (0x30)
            temp[i++] = '0' + (val % 10);
            
            // Loại bỏ chữ số cuối cùng
            val /= 10;
        }
        
        // Đảo ngược mảng temp và ghi vào buffer
        // Ví dụ: temp = ['3', '2', '1'] -> buf = "123"
        while (i > 0)
        {
            *buf++ = temp[--i];
        }
    }
    
    // Thêm null terminator để kết thúc chuỗi
    *buf = '\0';
    
    // Trả về con trỏ đến vị trí sau ký tự cuối (để có thể nối chuỗi)
    return buf;
}

/*
 * ============================================================================
 * HÀM: int_x10_to_str
 * ============================================================================
 * Mô tả: Chuyển số nguyên nhân 10 thành chuỗi số thực (1 chữ số thập phân)
 * 
 * Tham số:
 * - val_x10: Số nguyên đã nhân 10 (ví dụ: 256 = 25.6)
 * - buf: Con trỏ đến buffer để lưu chuỗi kết quả
 * 
 * Trả về: Con trỏ đến vị trí sau ký tự cuối cùng (trước '\0')
 * 
 * Ví dụ:
 * int_x10_to_str(256, buf)  -> "25.6"
 * int_x10_to_str(-123, buf) -> "-12.3"
 * int_x10_to_str(5, buf)    -> "0.5"
 * int_x10_to_str(0, buf)    -> "0.0"
 * 
 * Thuật toán:
 * 1. Xử lý dấu âm (nếu có)
 * 2. Lấy giá trị tuyệt đối
 * 3. Tách phần nguyên (val_x10 / 10) và phần thập phân (val_x10 % 10)
 * 4. Chuyển phần nguyên thành chuỗi (giống uint_to_str)
 * 5. Thêm dấu '.' và chữ số thập phân
 * 
 * Lý do cần hàm này:
 * - 8051 không có FPU, không thể dùng float
 * - Lưu nhiệt độ/độ ẩm dưới dạng int nhân 10 để tiết kiệm bộ nhớ
 * - Cần hiển thị dạng số thực (25.6°C) cho người dùng
 * 
 * Mối liên hệ:
 * - Được gọi bởi uart_protocol.c để chuyển temp, humid thành chuỗi JSON
 * - Được gọi bởi display.c để hiển thị nhiệt độ/độ ẩm lên LCD
 * ============================================================================
 */
char *int_x10_to_str(int val_x10, char *buf)
{
    unsigned int abs_val;  // Giá trị tuyệt đối
    char temp[6];          // Mảng tạm lưu các chữ số
    unsigned char i = 0;

    // ========================================================================
    // BƯỚC 1: XỬ LÝ DẤU ÂM
    // ========================================================================
    if (val_x10 < 0)
    {
        // Ghi dấu '-' vào buffer
        *buf++ = '-';
        
        // Lấy giá trị tuyệt đối
        // Ví dụ: -123 -> 123
        abs_val = (unsigned int)(-val_x10);
    }
    else
    {
        abs_val = (unsigned int)val_x10;
    }

    // ========================================================================
    // BƯỚC 2: XỬ LÝ PHẦN NGUYÊN
    // ========================================================================
    // Phần nguyên = abs_val / 10
    // Ví dụ: 256 / 10 = 25
    
    if (abs_val / 10 == 0)
    {
        // Trường hợp đặc biệt: phần nguyên = 0
        // Ví dụ: 5 / 10 = 0 -> ghi "0"
        *buf++ = '0';
    }
    else
    {
        // Tách phần nguyên
        unsigned int int_part = abs_val / 10;
        
        // Tách từng chữ số của phần nguyên (giống uint_to_str)
        while (int_part > 0)
        {
            temp[i++] = '0' + (int_part % 10);
            int_part /= 10;
        }
        
        // Đảo ngược và ghi vào buffer
        while (i > 0)
        {
            *buf++ = temp[--i];
        }
    }

    // ========================================================================
    // BƯỚC 3: THÊM DẤU CHẤM VÀ PHẦN THẬP PHÂN
    // ========================================================================
    // Thêm dấu chấm thập phân
    *buf++ = '.';
    
    // Thêm chữ số thập phân (abs_val % 10)
    // Ví dụ: 256 % 10 = 6 -> ghi '6'
    *buf++ = '0' + (abs_val % 10);
    
    // Thêm null terminator
    *buf = '\0';
    
    // Trả về con trỏ đến vị trí sau ký tự cuối
    return buf;
}
