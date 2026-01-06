/*
 * ============================================================================
 * DISPLAY MODULE - display.c
 * ============================================================================
 * Mô tả: Module xử lý logic hiển thị dữ liệu lên màn hình LCD 16x2
 * Chức năng:
 *   - Hiển thị dữ liệu cảm biến theo 4 chế độ khác nhau
 *   - Xử lý chế độ Auto (tự động chuyển đổi màn hình)
 *   - Hiển thị thông báo lỗi cảm biến
 *   - Format dữ liệu để vừa với màn hình 16 ký tự
 * 
 * Các chế độ hiển thị:
 * - Mode 0: Nhiệt độ + Độ ẩm (2 dòng)
 * - Mode 1: PM2.5 + Chất lượng không khí
 * - Mode 2: Gas + Mức độ nguy hiểm
 * - Mode 3: Auto (tự động chuyển đổi 3 chế độ trên)
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi main.c mỗi 500ms để cập nhật màn hình
 * - Sử dụng lcd.c để ghi dữ liệu lên LCD
 * - Sử dụng utils.c để chuyển số thành chuỗi
 * - Sử dụng delay.c để tạo độ trễ khi xóa màn hình
 * ============================================================================
 */

#include "display.h"
#include "../drivers/lcd.h"
#include "../drivers/delay.h"
#include "utils.h"

// Biến đếm màn hình cho chế độ Auto
// Tăng dần từ 0-11, mỗi 4 lần đếm = 1 màn hình (4 × 500ms = 2 giây)
static unsigned char screen_counter = 0;

/*
 * ============================================================================
 * HÀM: Get_AirQuality_Status
 * ============================================================================
 * Mô tả: Chuyển đổi giá trị PM2.5 thành mô tả chất lượng không khí
 * Tham số: pm25 - Nồng độ bụi PM2.5 (µg/m³)
 * Trả về: Con trỏ đến chuỗi mô tả chất lượng không khí
 * 
 * Bảng phân loại (theo tiêu chuẩn WHO):
 * - 0-10:    Excellent (Xuất sắc)
 * - 11-25:   Good (Tốt)
 * - 26-50:   Moderate (Trung bình)
 * - 51-70:   Poor (Kém)
 * - 71-150:  Bad (Xấu)
 * - >150:    Hazardous (Nguy hại)
 * 
 * Lưu ý:
 * - Hàm static: chỉ dùng trong file này
 * - Trả về con trỏ đến chuỗi hằng (ROM), không cần free
 * 
 * Mối liên hệ:
 * - Được gọi bởi Display_Update_Mode() khi hiển thị chế độ PM2.5
 * ============================================================================
 */
static char* Get_AirQuality_Status(unsigned int pm25)
{
    if (pm25 <= 10) return "Excellent";
    else if (pm25 <= 25) return "Good";
    else if (pm25 <= 50) return "Moderate";
    else if (pm25 <= 70) return "Poor";
    else if (pm25 <= 150) return "Bad";
    else return "Hazardous";
}

/*
 * ============================================================================
 * HÀM: Display_Update_Mode
 * ============================================================================
 * Mô tả: Cập nhật hiển thị LCD theo chế độ được chọn
 * 
 * Tham số:
 * - temp_x10: Nhiệt độ nhân 10 (ví dụ: 256 = 25.6°C)
 * - humid_x10: Độ ẩm nhân 10 (ví dụ: 653 = 65.3%)
 * - pm25: Nồng độ bụi PM2.5 (µg/m³)
 * - gas: Nồng độ khí gas (ppm)
 * - mode: Chế độ hiển thị (0=Temp/Humid, 1=PM2.5, 2=Gas, 3=Auto)
 * - err_aht10: Cờ lỗi cảm biến AHT10 (0=OK, 1=Error)
 * - err_pm25: Cờ lỗi cảm biến PM2.5 (0=OK, 1=Error)
 * - err_gas: Cờ lỗi cảm biến Gas (0=OK, 1=Error)
 * 
 * Chi tiết hoạt động:
 * 1. Xác định chế độ hiển thị thực tế (xử lý chế độ Auto)
 * 2. Xây dựng nội dung 2 dòng LCD (16 ký tự/dòng)
 * 3. Thêm khoảng trắng để đủ 16 ký tự (xóa ký tự cũ)
 * 4. Ghi 2 dòng lên LCD
 * 
 * Chế độ Auto (mode = 3):
 * - Tự động chuyển đổi giữa 3 chế độ (0, 1, 2)
 * - Mỗi chế độ hiển thị 2 giây (4 lần gọi × 500ms)
 * - Chu kỳ: Temp/Humid (2s) -> PM2.5 (2s) -> Gas (2s) -> lặp lại
 * 
 * Mối liên hệ:
 * - Được gọi từ main.c mỗi 500ms
 * - Gọi LCD_SetCursor() và LCD_Print() để hiển thị
 * - Gọi int_x10_to_str() và uint_to_str() để format số
 * ============================================================================
 */
void Display_Update_Mode(int temp_x10, int humid_x10, unsigned int pm25, unsigned int gas, 
                         unsigned char mode, unsigned char err_aht10, unsigned char err_pm25, 
                         unsigned char err_gas)
{
    // Buffer lưu nội dung 2 dòng LCD (16 ký tự + null terminator)
    // Dùng XRAM vì IRAM của 8051 chỉ có 128 bytes
    char xdata line1[17];
    char xdata line2[17];
    
    // Con trỏ dùng để xây dựng chuỗi từng ký tự
    char *p;
    
    // Con trỏ đến chuỗi trạng thái chất lượng không khí
    char *status;
    
    // Chế độ hiển thị thực tế (sau khi xử lý Auto)
    unsigned char actual_mode;
    
    // Lưu chế độ hiển thị lần trước (để phát hiện thay đổi)
    // static: giữ giá trị giữa các lần gọi hàm
    // 0xFF: giá trị khởi tạo (không trùng với mode 0-2)
    static unsigned char last_mode = 0xFF;

    // ========================================================================
    // BƯỚC 1: XÁC ĐỊNH CHẾ ĐỘ HIỂN THỊ THỰC TẾ
    // ========================================================================
    if (mode == 3)
    {
        // Chế độ Auto: tự động chuyển đổi màn hình
        
        // Tăng bộ đếm màn hình
        // Hàm này được gọi mỗi 500ms, nên:
        // - screen_counter = 0-3: hiển thị mode 0 (2 giây)
        // - screen_counter = 4-7: hiển thị mode 1 (2 giây)
        // - screen_counter = 8-11: hiển thị mode 2 (2 giây)
        screen_counter++;
        
        // Reset bộ đếm về 0 sau khi đạt 12 (6 giây)
        if (screen_counter >= 12) screen_counter = 0;
        
        // Tính chế độ thực tế: 0-3 -> 0, 4-7 -> 1, 8-11 -> 2
        actual_mode = screen_counter / 4;
    }
    else
    {
        // Chế độ thủ công: dùng mode được chọn
        actual_mode = mode;
        
        // Reset bộ đếm Auto (để khi chuyển sang Auto, bắt đầu từ đầu)
        screen_counter = 0;
    }
    
    // ========================================================================
    // BƯỚC 2: XÓA MÀN HÌNH KHI CHUYỂN CHẾ ĐỘ
    // ========================================================================
    // Chỉ xóa màn hình khi chế độ thay đổi (tránh nhấp nháy)
    if (actual_mode != last_mode) {
        // Gửi lệnh Clear Display (0x01) đến LCD
        LCD_WriteCommand(0x01);
        
        // Chờ LCD hoàn tất lệnh xóa (cần 1.52ms)
        delay_ms(2);
        
        // Lưu chế độ hiện tại để so sánh lần sau
        last_mode = actual_mode;
    }

    // ========================================================================
    // BƯỚC 3: XÂY DỰNG NỘI DUNG HIỂN THỊ THEO CHẾ ĐỘ
    // ========================================================================
    
    // ------------------------------------------------------------------------
    // CHẾ ĐỘ 0: NHIỆT ĐỘ VÀ ĐỘ ẨM
    // ------------------------------------------------------------------------
    // Dòng 1: T:25.6C HOT/COLD/OK
    // Dòng 2: H:65.3% WET/DRY/OK
    if (actual_mode == 0)
    {
        // Xây dựng dòng 1: Nhiệt độ
        p = line1;
        *p++ = 'T'; *p++ = ':';  // Nhãn "T:"
        
        if (err_aht10) {
            // Nếu có lỗi cảm biến, hiển thị "ERR"
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            // Chuyển nhiệt độ thành chuỗi (ví dụ: "25.6")
            p = int_x10_to_str(temp_x10, p);
            *p++ = 'C';  // Đơn vị độ C
        }
        
        *p++ = ' ';  // Khoảng trắng phân cách
        
        // Thêm trạng thái nhiệt độ
        if (!err_aht10) {
            if (temp_x10 > 350) {  // > 35°C
                *p++ = 'H'; *p++ = 'O'; *p++ = 'T';  // "HOT"
            }
            else if (temp_x10 < 100) {  // < 10°C
                *p++ = 'C'; *p++ = 'O'; *p++ = 'L'; *p++ = 'D';  // "COLD"
            }
            else {
                *p++ = 'O'; *p++ = 'K';  // "OK"
            }
        }
        *p = '\0';  // Kết thúc chuỗi

        // Xây dựng dòng 2: Độ ẩm
        p = line2;
        *p++ = 'H'; *p++ = ':';  // Nhãn "H:"
        
        if (err_aht10) {
            // Nếu có lỗi cảm biến, hiển thị "ERR"
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            // Chuyển độ ẩm thành chuỗi (ví dụ: "65.3")
            p = int_x10_to_str(humid_x10, p);
            *p++ = '%';  // Đơn vị %
        }
        
        *p++ = ' ';  // Khoảng trắng phân cách
        
        // Thêm trạng thái độ ẩm
        if (!err_aht10) {
            if (humid_x10 > 800) {  // > 80%
                *p++ = 'W'; *p++ = 'E'; *p++ = 'T';  // "WET" (ẩm ướt)
            }
            else if (humid_x10 < 300) {  // < 30%
                *p++ = 'D'; *p++ = 'R'; *p++ = 'Y';  // "DRY" (khô)
            }
            else {
                *p++ = 'O'; *p++ = 'K';  // "OK"
            }
        }
        *p = '\0';  // Kết thúc chuỗi
    }
    // ------------------------------------------------------------------------
    // CHẾ ĐỘ 1: NỒNG ĐỘ BỤI PM2.5
    // ------------------------------------------------------------------------
    // Dòng 1: PM2.5: 35 ug/m3
    // Dòng 2: Good/Moderate/Poor/...
    else if (actual_mode == 1)
    {
        // Xây dựng dòng 1: Nồng độ PM2.5
        p = line1;
        // Nhãn "PM2.5: "
        *p++ = 'P'; *p++ = 'M'; *p++ = '2'; *p++ = '.'; *p++ = '5'; *p++ = ':'; *p++ = ' ';
        
        if (err_pm25) {
            // Nếu có lỗi cảm biến, hiển thị "ERR"
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            // Chuyển PM2.5 thành chuỗi (ví dụ: "35")
            p = uint_to_str(pm25, p);
            // Thêm đơn vị "ug/m3" (µg/m³)
            *p++ = ' '; *p++ = 'u'; *p++ = 'g'; *p++ = '/'; *p++ = 'm'; *p++ = '3';
        }
        *p = '\0';  // Kết thúc chuỗi

        // Xây dựng dòng 2: Chất lượng không khí
        p = line2;
        if (err_pm25) {
            // Nếu có lỗi, hiển thị "SENSOR ERROR"
            *p++ = 'S'; *p++ = 'E'; *p++ = 'N'; *p++ = 'S'; *p++ = 'O'; *p++ = 'R';
            *p++ = ' '; *p++ = 'E'; *p++ = 'R'; *p++ = 'R'; *p++ = 'O'; *p++ = 'R';
        } else {
            // Lấy chuỗi trạng thái (Excellent/Good/Moderate/Poor/Bad/Hazardous)
            status = Get_AirQuality_Status(pm25);
            // Copy từng ký tự của chuỗi trạng thái
            while (*status) *p++ = *status++;
        }
        *p = '\0';  // Kết thúc chuỗi
    }
    // ------------------------------------------------------------------------
    // CHẾ ĐỘ 2: NỒNG ĐỘ KHÍ GAS
    // ------------------------------------------------------------------------
    // Dòng 1: Gas: 150 ppm
    // Dòng 2: Safe/Caution/Warning/DANGER!
    else if (actual_mode == 2)
    {
        // Xây dựng dòng 1: Nồng độ gas
        p = line1;
        // Nhãn "Gas: "
        *p++ = 'G'; *p++ = 'a'; *p++ = 's'; *p++ = ':'; *p++ = ' ';
        
        if (err_gas) {
            // Nếu có lỗi cảm biến, hiển thị "ERR"
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            // Chuyển gas thành chuỗi (ví dụ: "150")
            p = uint_to_str(gas, p);
            // Thêm đơn vị "ppm" (parts per million)
            *p++ = ' '; *p++ = 'p'; *p++ = 'p'; *p++ = 'm';
        }
        *p = '\0';  // Kết thúc chuỗi

        // Xây dựng dòng 2: Mức độ nguy hiểm
        p = line2;
        if (err_gas) {
            // Nếu có lỗi, hiển thị "SENSOR ERROR"
            *p++ = 'S'; *p++ = 'E'; *p++ = 'N'; *p++ = 'S'; *p++ = 'O'; *p++ = 'R';
            *p++ = ' '; *p++ = 'E'; *p++ = 'R'; *p++ = 'R'; *p++ = 'O'; *p++ = 'R';
        } else {
            // Phân loại mức độ nguy hiểm dựa trên nồng độ gas
            if (gas > 700)
            {
                // > 700 ppm: Rất nguy hiểm
                *p++ = 'D'; *p++ = 'A'; *p++ = 'N'; *p++ = 'G'; *p++ = 'E'; *p++ = 'R'; *p++ = '!';
            }
            else if (gas > 400)
            {
                // 401-700 ppm: Cảnh báo
                *p++ = 'W'; *p++ = 'a'; *p++ = 'r'; *p++ = 'n'; *p++ = 'i'; *p++ = 'n'; *p++ = 'g';
            }
            else if (gas > 200)
            {
                // 201-400 ppm: Thận trọng
                *p++ = 'C'; *p++ = 'a'; *p++ = 'u'; *p++ = 't'; *p++ = 'i'; *p++ = 'o'; *p++ = 'n';
            }
            else
            {
                // 0-200 ppm: An toàn
                *p++ = 'S'; *p++ = 'a'; *p++ = 'f'; *p++ = 'e';
            }
        }
        *p = '\0';  // Kết thúc chuỗi
    }
    // ------------------------------------------------------------------------
    // CHẾ ĐỘ MẶC ĐỊNH: HIỂN THỊ TẤT CẢ (COMPACT)
    // ------------------------------------------------------------------------
    // Dòng 1: T:25.6 H:65.3
    // Dòng 2: P:35 G:150
    // Chế độ này không bao giờ được dùng trong thực tế
    // (chỉ là fallback nếu actual_mode không phải 0, 1, 2)
    else
    {
        // Xây dựng dòng 1: Nhiệt độ + Độ ẩm (compact)
        p = line1;
        *p++ = 'T'; *p++ = ':';  // Nhãn "T:"
        if (err_aht10) {
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            p = int_x10_to_str(temp_x10, p);
        }
        *p++ = ' '; *p++ = 'H'; *p++ = ':';  // Nhãn "H:"
        if (err_aht10) {
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            p = int_x10_to_str(humid_x10, p);
        }
        *p = '\0';  // Kết thúc chuỗi

        // Xây dựng dòng 2: PM2.5 + Gas (compact)
        p = line2;
        *p++ = 'P'; *p++ = ':';  // Nhãn "P:"
        if (err_pm25) {
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            p = uint_to_str(pm25, p);
        }
        *p++ = ' '; *p++ = 'G'; *p++ = ':';  // Nhãn "G:"
        if (err_gas) {
            *p++ = 'E'; *p++ = 'R'; *p++ = 'R';
        } else {
            p = uint_to_str(gas, p);
        }
        *p = '\0';  // Kết thúc chuỗi
    }

    // ========================================================================
    // BƯỚC 4: THÊM KHOẢNG TRẮNG ĐỂ ĐỦ 16 KÝ TỰ
    // ========================================================================
    // Lý do: Xóa ký tự cũ còn sót lại trên LCD
    // Ví dụ: "Temperature" (11 ký tự) -> "Temperature     " (16 ký tự)
    {
        unsigned char len1 = 0, len2 = 0;
        char *ptr;
        
        // Đếm độ dài chuỗi line1
        ptr = line1;
        while (*ptr) { len1++; ptr++; }
        
        // Thêm khoảng trắng cho đủ 16 ký tự
        while (len1 < 16) {
            line1[len1++] = ' ';
        }
        line1[16] = '\0';  // Kết thúc chuỗi
        
        // Đếm độ dài chuỗi line2
        ptr = line2;
        while (*ptr) { len2++; ptr++; }
        
        // Thêm khoảng trắng cho đủ 16 ký tự
        while (len2 < 16) {
            line2[len2++] = ' ';
        }
        line2[16] = '\0';  // Kết thúc chuỗi
    }
    
    // ========================================================================
    // BƯỚC 5: HIỂN THỊ LÊN LCD
    // ========================================================================
    // Di chuyển con trỏ đến dòng 0, cột 0
    LCD_SetCursor(0, 0);
    // Hiển thị dòng 1
    LCD_Print(line1);
    
    // Di chuyển con trỏ đến dòng 1, cột 0
    LCD_SetCursor(1, 0);
    // Hiển thị dòng 2
    LCD_Print(line2);
}

/*
 * ============================================================================
 * HÀM: Display_SensorError
 * ============================================================================
 * Mô tả: Hiển thị màn hình lỗi cảm biến
 * 
 * Tham số:
 * - err_aht10: Cờ lỗi cảm biến AHT10 (0=OK, 1=Error)
 * - err_pm25: Cờ lỗi cảm biến PM2.5 (0=OK, 1=Error)
 * - err_gas: Cờ lỗi cảm biến Gas (0=OK, 1=Error)
 * 
 * Format hiển thị:
 * Dòng 1: SENSOR ERROR!
 * Dòng 2: T:O H:O P:X G:O
 *         (O=OK, X=Error)
 * 
 * Ví dụ:
 * - Tất cả OK: T:O H:O P:O G:O
 * - Lỗi PM2.5: T:O H:O P:X G:O
 * - Lỗi tất cả: T:X H:X P:X G:X
 * 
 * Lưu ý:
 * - Hàm này hiện không được sử dụng trong main.c
 * - Có thể dùng để debug hoặc hiển thị lỗi tập trung
 * 
 * Mối liên hệ:
 * - Có thể được gọi từ main.c khi phát hiện nhiều lỗi cảm biến
 * ============================================================================
 */
void Display_SensorError(unsigned char err_aht10, unsigned char err_pm25, unsigned char err_gas)
{
    char xdata line2[17];
    char *p;

    // Hiển thị dòng 1: "SENSOR ERROR!"
    LCD_SetCursor(0, 0);
    LCD_Print("SENSOR ERROR!   ");  // 16 ký tự (có khoảng trắng cuối)

    // Xây dựng dòng 2: Trạng thái từng cảm biến
    p = line2;
    
    // Temperature/Humidity (AHT10)
    *p++ = 'T'; *p++ = ':'; 
    *p++ = err_aht10 ? 'X' : 'O';  // X=Error, O=OK
    *p++ = ' ';
    
    *p++ = 'H'; *p++ = ':'; 
    *p++ = err_aht10 ? 'X' : 'O';  // Cùng cảm biến với T
    *p++ = ' ';
    
    // PM2.5
    *p++ = 'P'; *p++ = ':'; 
    *p++ = err_pm25 ? 'X' : 'O';
    *p++ = ' ';
    
    // Gas
    *p++ = 'G'; *p++ = ':'; 
    *p++ = err_gas ? 'X' : 'O';
    
    *p = '\0';  // Kết thúc chuỗi

    // Hiển thị dòng 2
    LCD_SetCursor(1, 0);
    LCD_Print(line2);
}
