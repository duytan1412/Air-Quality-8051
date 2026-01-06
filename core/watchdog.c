/*
 * ============================================================================
 * WATCHDOG TIMER MODULE - watchdog.c
 * ============================================================================
 * Mô tả: Module quản lý Watchdog Timer (WDT) của vi điều khiển 8051
 * Chức năng: Tự động reset hệ thống nếu phần mềm bị treo/lỗi
 * 
 * Nguyên lý hoạt động:
 * - Watchdog là bộ đếm thời gian chạy độc lập
 * - Nếu không được "cho ăn" (feed) định kỳ, nó sẽ reset vi điều khiển
 * - Đảm bảo hệ thống tự phục hồi khi gặp lỗi phần mềm
 * 
 * Chu kỳ Watchdog:
 * - Với thạch anh 11.0592MHz, chu kỳ WDT ≈ 16-17ms
 * - Phải gọi WDT_Feed() ít nhất mỗi 16ms để tránh reset
 * 
 * QUAN TRỌNG - TÍCH HỢP VỚI DELAY:
 * - Hàm delay_ms() trong delay.c đã tích hợp WDT_Feed()
 * - delay_ms() gọi WDT_Feed() mỗi 10ms trong vòng lặp
 * - Điều này cho phép delay bất kỳ thời gian nào (500ms, 2000ms, ...)
 *   mà không bị Watchdog reset
 * - Nếu không tích hợp, delay_ms(500) sẽ gây reset sau 16ms!
 * 
 * Mối liên hệ với các module khác:
 * - WDT_Init() được gọi 1 lần trong main() khi khởi động
 * - WDT_Feed() được gọi trong vòng lặp chính của main()
 * - WDT_Feed() được gọi bởi delay_ms() mỗi 10ms
 * - Nếu vòng lặp bị treo, WDT sẽ tự động reset hệ thống
 * ============================================================================
 */

#include "watchdog.h"

// Định nghĩa thanh ghi WDTRST (Watchdog Timer Reset) tại địa chỉ 0xA6
// Chỉ định nghĩa khi biên dịch với Keil C51 compiler
#ifdef __C51__
sfr WDTRST = 0xA6;
#endif

/*
 * ============================================================================
 * HÀM: WDT_Init
 * ============================================================================
 * Mô tả: Khởi tạo và bật Watchdog Timer
 * 
 * Chi tiết hoạt động:
 * - Ghi 2 giá trị đặc biệt (0x1E, 0xE1) vào thanh ghi WDTRST
 * - Thứ tự ghi phải chính xác: 0x1E trước, 0xE1 sau
 * - Sau khi khởi tạo, WDT bắt đầu đếm ngay lập tức
 * 
 * Lưu ý:
 * - Phải gọi WDT_Feed() định kỳ sau khi khởi tạo
 * - Nếu không feed, hệ thống sẽ reset sau ~16ms
 * - Không thể tắt WDT sau khi đã bật (chỉ reset mới tắt)
 * 
 * Mối liên hệ:
 * - Được gọi 1 lần duy nhất trong main() khi khởi động hệ thống
 * - Sau khi gọi hàm này, phải gọi WDT_Feed() định kỳ trong vòng lặp
 * ============================================================================
 */
void WDT_Init(void)
{
    // Ghi giá trị 0x1E vào thanh ghi WDTRST
    // Đây là bước 1 của chuỗi khởi tạo WDT
    WDTRST = 0x1E;
    
    // Ghi giá trị 0xE1 vào thanh ghi WDTRST
    // Đây là bước 2 của chuỗi khởi tạo WDT
    // Sau 2 lệnh này, WDT được bật và bắt đầu đếm
    WDTRST = 0xE1;
}

/*
 * ============================================================================
 * HÀM: WDT_Feed
 * ============================================================================
 * Mô tả: "Cho Watchdog ăn" - Reset bộ đếm WDT về 0
 * 
 * Chi tiết hoạt động:
 * - Ghi 2 giá trị đặc biệt (0x1E, 0xE1) vào thanh ghi WDTRST
 * - Thứ tự ghi phải chính xác: 0x1E trước, 0xE1 sau
 * - Bộ đếm WDT được reset về 0, bắt đầu đếm lại từ đầu
 * 
 * Lưu ý:
 * - Phải gọi hàm này ít nhất mỗi 16ms (với thạch anh 11.0592MHz)
 * - Nếu quên gọi hoặc vòng lặp bị treo, WDT sẽ reset vi điều khiển
 * - Đây là cơ chế bảo vệ tự động khỏi lỗi phần mềm
 * 
 * Ví dụ sử dụng:
 * while(1) {
 *     WDT_Feed();        // Cho WDT ăn đầu vòng lặp
 *     // Xử lý công việc...
 *     delay_ms(500);     // AN TOÀN! delay_ms() đã tích hợp WDT_Feed()
 * }
 * 
 * Lưu ý: delay_ms() trong delay.c đã gọi WDT_Feed() mỗi 10ms
 * nên có thể delay bất kỳ thời gian nào mà không bị reset.
 * 
 * Mối liên hệ:
 * - Được gọi trong vòng lặp chính của main() (đầu mỗi vòng lặp)
 * - Đảm bảo hệ thống không bị reset khi hoạt động bình thường
 * - Nếu vòng lặp bị treo (do lỗi), WDT sẽ tự động reset hệ thống
 * ============================================================================
 */
void WDT_Feed(void)
{
    // Ghi giá trị 0x1E vào thanh ghi WDTRST
    // Đây là bước 1 của chuỗi reset WDT
    WDTRST = 0x1E;
    
    // Ghi giá trị 0xE1 vào thanh ghi WDTRST
    // Đây là bước 2 của chuỗi reset WDT
    // Sau 2 lệnh này, bộ đếm WDT được reset về 0
    WDTRST = 0xE1;
}
