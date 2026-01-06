/*
 * ============================================================================
 * UTILITY FUNCTIONS HEADER FILE - utils.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm tiện ích
 * Mục đích: Cung cấp interface để chuyển đổi số thành chuỗi
 * ============================================================================
 */

#ifndef __UTILS_H__
#define __UTILS_H__

/*
 * Hàm: uint_to_str
 * Mô tả: Chuyển số nguyên không dấu thành chuỗi
 * Tham số:
 *   - val: Số nguyên cần chuyển (0 - 65535)
 *   - buf: Con trỏ đến buffer để lưu chuỗi
 * Trả về: Con trỏ đến vị trí sau ký tự cuối (để nối chuỗi)
 * Ví dụ: uint_to_str(123, buf) -> "123"
 */
char *uint_to_str(unsigned int val, char *buf);

/*
 * Hàm: int_x10_to_str
 * Mô tả: Chuyển số nguyên nhân 10 thành chuỗi số thực (1 chữ số thập phân)
 * Tham số:
 *   - val_x10: Số nguyên đã nhân 10 (ví dụ: 256 = 25.6)
 *   - buf: Con trỏ đến buffer để lưu chuỗi
 * Trả về: Con trỏ đến vị trí sau ký tự cuối (để nối chuỗi)
 * Ví dụ: int_x10_to_str(256, buf) -> "25.6"
 * Lưu ý: Hỗ trợ số âm (ví dụ: -123 -> "-12.3")
 */
char *int_x10_to_str(int val_x10, char *buf);

#endif
