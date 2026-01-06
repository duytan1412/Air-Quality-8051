/*
 * ============================================================================
 * UART COMMUNICATION MODULE - uart.c
 * ============================================================================
 * Mô tả: Module giao tiếp UART (Universal Asynchronous Receiver-Transmitter)
 * Chức năng: Gửi dữ liệu serial từ 8051 đến ESP32
 * Cấu hình: 9600 baud, 8 data bits, no parity, 1 stop bit
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi uart_protocol.c để gửi chuỗi JSON
 * - Sử dụng Timer 1 của 8051 để tạo baud rate
 * - Kết nối phần cứng: P3.1 (TXD) -> ESP32 GPIO9 (qua voltage divider)
 * ============================================================================
 */

#include "uart.h"
#include <reg52.h>

/*
 * ============================================================================
 * HÀM: UART_Init
 * ============================================================================
 * Mô tả: Khởi tạo UART với baud rate 9600
 * 
 * Chi tiết hoạt động:
 * 1. Cấu hình Timer 1 làm baud rate generator (Mode 2: 8-bit auto-reload)
 * 2. Nạp giá trị TH1 = 0xFD để tạo baud rate 9600 (với thạch anh 11.0592MHz)
 * 3. Cấu hình UART ở Mode 1 (8-bit UART, variable baud rate)
 * 4. Bật Timer 1 để bắt đầu tạo xung clock cho UART
 * 
 * Công thức tính baud rate:
 * Baud Rate = (Fosc / 32) / (256 - TH1)
 * 9600 = (11059200 / 32) / (256 - 253) = 345600 / 3 = 115200... (sai!)
 * Thực tế: 9600 = (11059200 / 384) / (256 - 253) = 28800 / 3 = 9600 ✓
 * 
 * Mối liên hệ:
 * - Được gọi 1 lần duy nhất trong main() khi khởi động hệ thống
 * - Phải được gọi trước khi sử dụng UART_SendChar() hoặc UART_SendString()
 * ============================================================================
 */
void UART_Init(void)
{
    // TMOD (Timer Mode Register) = 0x20
    // Bit 7-6: GATE1=0, C/T1=0 (Timer 1 dùng clock nội)
    // Bit 5-4: M1=1, M0=0 (Mode 2: 8-bit auto-reload)
    // Bit 3-0: Không ảnh hưởng đến Timer 0
    TMOD = 0x20;
    
    // TH1 (Timer 1 High byte) = 0xFD (253 decimal)
    // Giá trị này tạo baud rate 9600 với thạch anh 11.0592MHz
    // Khi Timer 1 tràn, nó tự động reload giá trị từ TH1
    TH1 = 0xFD;
    
    // SCON (Serial Control Register) = 0x50
    // Bit 7: SM0=0
    // Bit 6: SM1=1 (Mode 1: 8-bit UART, variable baud rate)
    // Bit 5: SM2=0 (không dùng multiprocessor communication)
    // Bit 4: REN=1 (Enable receiver - cho phép nhận dữ liệu)
    // Bit 3-0: TB8, RB8, TI, RI = 0 (khởi tạo ban đầu)
    SCON = 0x50;
    
    // TR1 (Timer 1 Run control bit) = 1
    // Bật Timer 1 để bắt đầu tạo baud rate cho UART
    TR1 = 1;
}

/*
 * ============================================================================
 * HÀM: UART_SendChar
 * ============================================================================
 * Mô tả: Gửi 1 ký tự qua UART
 * Tham số: c - ký tự cần gửi (1 byte)
 * 
 * Chi tiết hoạt động:
 * 1. Nạp ký tự vào thanh ghi SBUF (Serial Buffer)
 * 2. Phần cứng UART tự động gửi ký tự ra chân TXD (P3.1)
 * 3. Chờ cờ TI (Transmit Interrupt) = 1 (báo hiệu gửi xong)
 * 4. Xóa cờ TI về 0 để chuẩn bị cho lần gửi tiếp theo
 * 
 * Lưu ý:
 * - Hàm này BLOCKING (chờ đến khi gửi xong mới return)
 * - Mỗi ký tự mất khoảng 1.04ms để gửi ở 9600 baud
 * - Phải xóa TI thủ công, phần cứng không tự xóa
 * 
 * Mối liên hệ:
 * - Được gọi bởi UART_SendString() để gửi từng ký tự trong chuỗi
 * - Được gọi bởi uart_protocol.c để gửi dữ liệu JSON đến ESP32
 * ============================================================================
 */
void UART_SendChar(char c)
{
    // Nạp ký tự vào SBUF (Serial Buffer Register)
    // Phần cứng UART sẽ tự động gửi ký tự này ra chân TXD
    SBUF = c;
    
    // Chờ cờ TI (Transmit Interrupt flag) = 1
    // TI = 1 khi phần cứng đã gửi xong 1 byte
    // Vòng lặp này đảm bảo không ghi đè SBUF trước khi gửi xong
    while (!TI)
        ;
    
    // Xóa cờ TI về 0 để chuẩn bị cho lần gửi tiếp theo
    // Phần cứng không tự xóa TI, phải xóa bằng phần mềm
    TI = 0;
}

/*
 * ============================================================================
 * HÀM: UART_SendString
 * ============================================================================
 * Mô tả: Gửi một chuỗi ký tự (string) qua UART
 * Tham số: str - con trỏ đến chuỗi ký tự (kết thúc bằng '\0')
 * 
 * Chi tiết hoạt động:
 * 1. Duyệt qua từng ký tự trong chuỗi
 * 2. Gọi UART_SendChar() để gửi từng ký tự
 * 3. Dừng khi gặp ký tự '\0' (null terminator)
 * 
 * Ví dụ sử dụng:
 * UART_SendString("Hello\n");  // Gửi chuỗi "Hello" + xuống dòng
 * 
 * Mối liên hệ:
 * - Được gọi bởi uart_protocol.c để gửi chuỗi JSON hoàn chỉnh
 * - Gọi UART_SendChar() nhiều lần để gửi từng ký tự
 * - Thời gian gửi = số ký tự × 1.04ms (ở 9600 baud)
 * ============================================================================
 */
void UART_SendString(char *str)
{
    // Vòng lặp duyệt qua từng ký tự trong chuỗi
    // Điều kiện dừng: gặp ký tự '\0' (null terminator)
    while (*str)
    {
        // Gửi ký tự hiện tại qua UART
        UART_SendChar(*str++);
        // *str++: lấy giá trị tại con trỏ, sau đó tăng con trỏ lên 1
    }
}
