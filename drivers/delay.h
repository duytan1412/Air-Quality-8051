/*
 * ============================================================================
 * DELAY HEADER FILE - delay.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm tạo độ trễ thời gian
 * Mục đích: Cung cấp interface để tạo delay cho các thao tác phần cứng
 * 
 * QUAN TRỌNG - WATCHDOG INTEGRATION:
 * - Hàm delay_ms() đã tích hợp WDT_Feed() bên trong
 * - Có thể delay bất kỳ thời gian nào mà không bị Watchdog reset
 * - delay_ms() gọi WDT_Feed() mỗi 10ms trong vòng lặp
 * ============================================================================
 */

#ifndef __DELAY_H__
#define __DELAY_H__

/*
 * Hàm: delay_us
 * Mô tả: Tạo độ trễ microsecond (µs)
 * Tham số: us - Số microsecond cần delay (1-65535)
 * Trả về: Không
 * Lưu ý:
 *   - Timing phụ thuộc vào tần số thạch anh (11.0592 MHz)
 *   - Không chính xác 100%
 *   - Hàm blocking
 *   - KHÔNG tích hợp WDT_Feed() (delay quá ngắn)
 * Ví dụ: delay_us(5);  // Delay 5 µs
 */
void delay_us(unsigned int us);

/*
 * Hàm: delay_ms
 * Mô tả: Tạo độ trễ millisecond (ms) với tích hợp Watchdog Feed
 * Tham số: ms - Số millisecond cần delay (1-65535)
 * Trả về: Không
 * Lưu ý:
 *   - Timing phụ thuộc vào tần số thạch anh (11.0592 MHz)
 *   - Hàm blocking
 *   - ĐÃ TÍCH HỢP WDT_Feed() mỗi 10ms => delay dài vẫn an toàn!
 *   - Có thể delay 500ms, 2000ms, ... mà không bị Watchdog reset
 * Ví dụ: delay_ms(500);  // Delay 500 ms - AN TOÀN!
 */
void delay_ms(unsigned int ms);

#endif
