/*
 * ============================================================================
 * WATCHDOG TIMER HEADER FILE - watchdog.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm quản lý Watchdog Timer
 * Mục đích: Cung cấp interface để khởi tạo và feed Watchdog
 * 
 * QUAN TRỌNG - TÍCH HỢP VỚI DELAY:
 * - Hàm delay_ms() trong delay.c đã tích hợp WDT_Feed()
 * - delay_ms() gọi WDT_Feed() mỗi 10ms trong vòng lặp
 * - Điều này cho phép delay bất kỳ thời gian nào mà không bị reset
 * ============================================================================
 */

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

/*
 * Hàm: WDT_Init
 * Mô tả: Khởi tạo và bật Watchdog Timer
 * Tham số: Không
 * Trả về: Không
 * Lưu ý: 
 *   - Chỉ gọi 1 lần khi khởi động hệ thống
 *   - Sau khi gọi, phải feed WDT định kỳ (< 16ms)
 *   - Không thể tắt WDT sau khi đã bật
 */
void WDT_Init(void);

/*
 * Hàm: WDT_Feed
 * Mô tả: "Cho Watchdog ăn" - Reset bộ đếm WDT về 0
 * Tham số: Không
 * Trả về: Không
 * Lưu ý:
 *   - Phải gọi ít nhất mỗi 16ms (với thạch anh 11.0592MHz)
 *   - Nếu không gọi, WDT sẽ reset vi điều khiển
 *   - Thường gọi ở đầu vòng lặp chính
 *   - delay_ms() đã tích hợp WDT_Feed() nên delay dài vẫn an toàn
 */
void WDT_Feed(void);

#endif
