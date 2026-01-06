/*
 * ============================================================================
 * UART HEADER FILE - uart.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm giao tiếp UART
 * Mục đích: Cung cấp interface để các module khác sử dụng UART
 * ============================================================================
 */

#ifndef __UART_H__
#define __UART_H__

/*
 * Hàm: UART_Init
 * Mô tả: Khởi tạo UART với baud rate 9600
 * Tham số: Không
 * Trả về: Không
 * Lưu ý: Phải gọi hàm này trước khi sử dụng UART
 */
void UART_Init(void);

/*
 * Hàm: UART_SendChar
 * Mô tả: Gửi 1 ký tự qua UART
 * Tham số: c - ký tự cần gửi
 * Trả về: Không
 * Lưu ý: Hàm blocking, chờ đến khi gửi xong
 */
void UART_SendChar(char c);

/*
 * Hàm: UART_SendString
 * Mô tả: Gửi một chuỗi ký tự qua UART
 * Tham số: str - con trỏ đến chuỗi (kết thúc bằng '\0')
 * Trả về: Không
 * Lưu ý: Gửi từng ký tự cho đến khi gặp '\0'
 */
void UART_SendString(char *str);

#endif
