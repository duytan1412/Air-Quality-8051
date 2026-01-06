/*
 * ============================================================================
 * DELAY MODULE - delay.c
 * ============================================================================
 * Mô tả: Module tạo độ trễ thời gian cho các thao tác phần cứng
 * Chức năng: Tạo delay microsecond và millisecond
 * 
 * Lưu ý quan trọng:
 * - Timing phụ thuộc vào tần số thạch anh (11.0592 MHz)
 * - Timing có thể không chính xác 100% do overhead của vòng lặp
 * - Không nên dùng cho timing chính xác cao (dùng Timer thay thế)
 * 
 * QUAN TRỌNG - WATCHDOG INTEGRATION:
 * - Hàm delay_ms() có tích hợp WDT_Feed() để tránh Watchdog reset
 * - Với thạch anh 11.0592MHz, Watchdog timeout ~16-17ms
 * - Nếu delay > 16ms mà không feed WDT, hệ thống sẽ bị reset
 * - Giải pháp: Gọi WDT_Feed() mỗi 1ms trong vòng lặp delay (RẤT AN TOÀN)
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi i2c.c để tạo timing cho giao thức I2C
 * - Được gọi bởi lcd.c để chờ LCD xử lý lệnh
 * - Được gọi bởi aht10.c để chờ cảm biến đo xong
 * - Được gọi bởi ads1115.c để chờ ADC chuyển đổi
 * - Được gọi bởi gp2y1014.c để điều khiển LED
 * - Được gọi bởi main.c để tạo chu kỳ vòng lặp
 * - Phụ thuộc vào watchdog.h để gọi WDT_Feed()
 * ============================================================================
 */

#include <intrins.h>  // Thư viện cho hàm _nop_() (No Operation)
#include "delay.h"
#include "../core/watchdog.h"  // Thư viện Watchdog để gọi WDT_Feed()

/*
 * ============================================================================
 * HÀM: delay_us
 * ============================================================================
 * Mô tả: Tạo độ trễ microsecond (µs)
 * Tham số: us - Số microsecond cần delay (1-65535)
 * 
 * Chi tiết hoạt động:
 * - Mỗi vòng lặp thực hiện 2 lệnh _nop_() (No Operation)
 * - Mỗi lệnh _nop_() mất 1 chu kỳ máy
 * - Với thạch anh 11.0592 MHz: 1 chu kỳ máy = 1/11.0592 µs ≈ 0.09 µs
 * - Thời gian thực tế có thể sai lệch do overhead của vòng lặp
 * 
 * Ví dụ sử dụng:
 * delay_us(5);   // Delay 5 microsecond (dùng trong I2C)
 * delay_us(280); // Delay 280 microsecond (dùng trong GP2Y1014)
 * 
 * Lưu ý:
 * - Không chính xác 100%, chỉ dùng cho timing không quan trọng
 * - Với delay chính xác cao, nên dùng Timer
 * - Hàm blocking: CPU không làm gì khác trong thời gian delay
 * 
 * Mối liên hệ:
 * - Được gọi bởi I2C_Start(), I2C_Stop(), I2C_WriteByte(), I2C_ReadByte()
 * - Được gọi bởi LCD_Enable() để tạo xung EN
 * - Được gọi bởi GP2Y1014_Read() để điều khiển LED
 * ============================================================================
 */
void delay_us(unsigned int us)
{
    // Vòng lặp chạy 'us' lần
    while (us--)
    {
        // Thực hiện 2 lệnh NOP (No Operation)
        // Mỗi lệnh NOP mất 1 chu kỳ máy
        // Tổng: 2 chu kỳ máy ≈ 0.18 µs (với 11.0592 MHz)
        _nop_();
        _nop_();
    }
}

/*
 * ============================================================================
 * HÀM: delay_ms
 * ============================================================================
 * Mô tả: Tạo độ trễ millisecond (ms) với tích hợp Watchdog Feed
 * Tham số: ms - Số millisecond cần delay (1-65535)
 * 
 * Chi tiết hoạt động:
 * - Vòng lặp ngoài chạy 'ms' lần (số millisecond)
 * - Vòng lặp trong chạy 120 lần (hiệu chỉnh để được ~1ms)
 * - Với thạch anh 11.0592 MHz, 120 vòng lặp ≈ 1ms
 * 
 * QUAN TRỌNG - WATCHDOG INTEGRATION:
 * - Watchdog timeout với thạch anh 11.0592MHz là ~16-17ms
 * - Nếu delay > 16ms mà không feed WDT => HỆ THỐNG BỊ RESET!
 * - Giải pháp: Gọi WDT_Feed() mỗi 1ms trong vòng lặp delay (RẤT AN TOÀN)
 * - Điều này cho phép delay bất kỳ thời gian nào mà không bị reset
 * 
 * Công thức tính:
 * - 1 vòng lặp trong ≈ 8.3 µs (ước tính)
 * - 120 vòng lặp ≈ 1000 µs = 1 ms
 * 
 * Ví dụ sử dụng:
 * delay_ms(2);    // Delay 2 millisecond (dùng trong LCD)
 * delay_ms(10);   // Delay 10 millisecond (dùng trong ADS1115)
 * delay_ms(80);   // Delay 80 millisecond (dùng trong AHT10)
 * delay_ms(500);  // Delay 500 millisecond (dùng trong main loop) - AN TOÀN!
 * delay_ms(2000); // Delay 2 giây (dùng cho màn hình chào) - AN TOÀN!
 * 
 * Lưu ý:
 * - Timing phụ thuộc vào tần số thạch anh
 * - Nếu đổi thạch anh, cần hiệu chỉnh lại giá trị 120
 * - Hàm blocking: CPU không làm gì khác trong thời gian delay
 * - Có thể delay bất kỳ thời gian nào nhờ tích hợp WDT_Feed()
 * 
 * Mối liên hệ:
 * - Được gọi bởi LCD_Init() để chờ LCD khởi động
 * - Được gọi bởi AHT10_Init() để chờ cảm biến ổn định
 * - Được gọi bởi AHT10_Read() để chờ cảm biến đo xong
 * - Được gọi bởi main() để tạo chu kỳ vòng lặp 500ms
 * - Được gọi bởi main() để chống dội phím (20ms)
 * - Gọi WDT_Feed() từ watchdog.c để tránh Watchdog reset
 * ============================================================================
 */
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    
    // Vòng lặp ngoài: chạy 'ms' lần (số millisecond)
    for (i = 0; i < ms; i++)
    {
        // ====================================================================
        // WATCHDOG FEED - QUAN TRỌNG! (ĐẶT Ở ĐẦU MỖI 1ms)
        // ====================================================================
        // Mỗi lần lặp vòng ngoài tương đương 1ms
        // Watchdog timeout ~16-17ms, nên cho ăn mỗi 1ms là RẤT AN TOÀN
        // Bằng cách này, dù delay_ms(500) hay delay_ms(5000), 
        // cứ sau mỗi 1ms Watchdog lại được reset về 0
        // => Không bao giờ bị đói và không bao giờ tự ý reset chip
        WDT_Feed();
        
        // Vòng lặp trong: chạy 120 lần để tạo ~1ms
        // Giá trị 120 được hiệu chỉnh thực nghiệm với thạch anh 11.0592 MHz
        for (j = 0; j < 120; j++)
        {
            // Vòng lặp rỗng: chỉ để tốn thời gian
            // Compiler có thể tối ưu hóa vòng lặp này
        }
    }
}
