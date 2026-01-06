/*
 * ============================================================================
 * LCD 16x2 DRIVER - lcd.c
 * ============================================================================
 * Mô tả: Driver điều khiển màn hình LCD 16x2 (HD44780 compatible)
 * Chế độ: 8-bit mode (sử dụng 8 chân data D0-D7)
 * 
 * Thông số LCD:
 * - Kích thước: 16 ký tự × 2 dòng
 * - Controller: HD44780 hoặc tương đương
 * - Chế độ: 8-bit parallel interface
 * - Font: 5×8 dots
 * 
 * Kết nối phần cứng:
 * - Data: P0.0-P0.7 (D0-D7)
 * - RS (Register Select): P2.6
 * - RW (Read/Write): P2.5
 * - EN (Enable): P2.7
 * 
 * Giao thức HD44780:
 * - RS=0: Gửi lệnh (command)
 * - RS=1: Gửi dữ liệu (character)
 * - RW=0: Ghi (write)
 * - RW=1: Đọc (read) - không dùng trong driver này
 * - EN: Xung enable để LCD đọc dữ liệu (HIGH->LOW)
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi display.c để hiển thị dữ liệu cảm biến
 * - Được gọi bởi main.c để hiển thị màn hình chào
 * - Sử dụng delay.c để tạo timing cho LCD
 * ============================================================================
 */

#include "lcd.h"
#include "delay.h"

/*
 * ============================================================================
 * HÀM: LCD_Enable
 * ============================================================================
 * Mô tả: Tạo xung enable để LCD đọc dữ liệu từ bus
 * 
 * Timing xung EN:
 *     EN: ___/‾‾‾\___________
 *         ^  ^  ^
 *         |  |  |
 *       LOW HIGH LOW
 *         1µs  50µs
 * 
 * Chi tiết hoạt động:
 * 1. Kéo EN lên HIGH (LCD bắt đầu đọc dữ liệu)
 * 2. Giữ HIGH trong 1µs (enable pulse width)
 * 3. Kéo EN xuống LOW (LCD latch dữ liệu)
 * 4. Chờ 50µs (LCD xử lý lệnh/dữ liệu)
 * 
 * Lưu ý:
 * - Hàm static: chỉ dùng trong file này
 * - Phải gọi sau khi đã set RS, RW và DATA
 * - Timing theo datasheet HD44780
 * 
 * Mối liên hệ:
 * - Được gọi bởi LCD_WriteCommand() sau khi set lệnh
 * - Được gọi bởi LCD_WriteData() sau khi set ký tự
 * ============================================================================
 */
static void LCD_Enable(void)
{
    // Kéo EN lên HIGH - LCD bắt đầu đọc dữ liệu từ bus
    LCD_EN = 1;
    
    // Giữ EN = HIGH trong 1µs (enable pulse width min = 450ns)
    delay_us(1);
    
    // Kéo EN xuống LOW - LCD latch dữ liệu vào thanh ghi
    LCD_EN = 0;
    
    // Chờ LCD xử lý lệnh/dữ liệu (execution time)
    // Hầu hết lệnh cần ~37µs, chờ 50µs để an toàn
    delay_us(50);
}

/*
 * ============================================================================
 * HÀM: LCD_WriteCommand
 * ============================================================================
 * Mô tả: Gửi lệnh (command) đến LCD
 * Tham số: cmd - Mã lệnh (8-bit)
 * 
 * Các lệnh thường dùng:
 * - 0x01: Clear Display (xóa màn hình)
 * - 0x02: Return Home (về đầu dòng 0)
 * - 0x06: Entry Mode Set (tự động tăng con trỏ)
 * - 0x0C: Display ON, Cursor OFF
 * - 0x0E: Display ON, Cursor ON
 * - 0x38: Function Set (8-bit, 2 lines, 5×8 font)
 * - 0x80: Set DDRAM Address (dòng 0, cột 0)
 * - 0xC0: Set DDRAM Address (dòng 1, cột 0)
 * 
 * Chi tiết hoạt động:
 * 1. Set RS = 0 (chế độ command)
 * 2. Set RW = 0 (chế độ write)
 * 3. Đặt mã lệnh lên bus DATA
 * 4. Tạo xung EN để LCD đọc lệnh
 * 
 * Ví dụ sử dụng:
 * LCD_WriteCommand(0x01);  // Xóa màn hình
 * LCD_WriteCommand(0x80);  // Di chuyển con trỏ về (0,0)
 * 
 * Mối liên hệ:
 * - Được gọi bởi LCD_Init() để khởi tạo LCD
 * - Được gọi bởi LCD_SetCursor() để di chuyển con trỏ
 * - Được gọi bởi display.c để xóa màn hình
 * ============================================================================
 */
void LCD_WriteCommand(unsigned char cmd)
{
    // Set RS = 0: Chế độ command (gửi lệnh)
    LCD_RS = 0;
    
    // Set RW = 0: Chế độ write (ghi dữ liệu)
    LCD_RW = 0;
    
    // Đặt mã lệnh lên bus DATA (P0)
    LCD_DATA = cmd;
    
    // Tạo xung EN để LCD đọc lệnh
    LCD_Enable();
}

/*
 * ============================================================================
 * HÀM: LCD_WriteData
 * ============================================================================
 * Mô tả: Gửi dữ liệu (ký tự) đến LCD
 * Tham số: dat - Mã ASCII của ký tự (8-bit)
 * 
 * Chi tiết hoạt động:
 * 1. Set RS = 1 (chế độ data)
 * 2. Set RW = 0 (chế độ write)
 * 3. Đặt mã ASCII lên bus DATA
 * 4. Tạo xung EN để LCD đọc ký tự
 * 5. LCD tự động tăng con trỏ sang vị trí tiếp theo
 * 
 * Ví dụ sử dụng:
 * LCD_WriteData('A');   // Hiển thị ký tự 'A'
 * LCD_WriteData(0x41);  // Hiển thị ký tự 'A' (mã ASCII)
 * 
 * Lưu ý:
 * - Phải gọi LCD_SetCursor() trước để chọn vị trí hiển thị
 * - LCD tự động tăng con trỏ sau mỗi ký tự (nếu đã set Entry Mode)
 * - Nếu con trỏ vượt quá 16 ký tự, sẽ tràn sang dòng tiếp theo
 * 
 * Mối liên hệ:
 * - Được gọi bởi LCD_Print() để hiển thị chuỗi
 * ============================================================================
 */
void LCD_WriteData(unsigned char dat)
{
    // Set RS = 1: Chế độ data (gửi ký tự)
    LCD_RS = 1;
    
    // Set RW = 0: Chế độ write (ghi dữ liệu)
    LCD_RW = 0;
    
    // Đặt mã ASCII lên bus DATA (P0)
    LCD_DATA = dat;
    
    // Tạo xung EN để LCD đọc ký tự
    LCD_Enable();
}

/*
 * ============================================================================
 * HÀM: LCD_Init
 * ============================================================================
 * Mô tả: Khởi tạo LCD theo quy trình chuẩn HD44780
 * 
 * Quy trình khởi tạo (theo datasheet HD44780):
 * 1. Chờ > 15ms sau khi bật nguồn
 * 2. Gửi lệnh 0x38 (Function Set) 3 lần
 * 3. Gửi lệnh 0x0C (Display ON, Cursor OFF)
 * 4. Gửi lệnh 0x06 (Entry Mode: tự động tăng con trỏ)
 * 5. Gửi lệnh 0x01 (Clear Display)
 * 
 * Lệnh 0x38 (Function Set):
 * - Bit 5-4: DL=1 (8-bit mode)
 * - Bit 3: N=1 (2 lines)
 * - Bit 2: F=0 (5×8 font)
 * 
 * Lệnh 0x0C (Display Control):
 * - Bit 3: D=1 (Display ON)
 * - Bit 2: C=0 (Cursor OFF)
 * - Bit 1: B=0 (Blink OFF)
 * 
 * Lệnh 0x06 (Entry Mode Set):
 * - Bit 1: I/D=1 (Increment: tự động tăng con trỏ)
 * - Bit 0: S=0 (No shift: không dịch màn hình)
 * 
 * Lệnh 0x01 (Clear Display):
 * - Xóa toàn bộ màn hình
 * - Đưa con trỏ về (0,0)
 * - Cần chờ 1.52ms để hoàn tất
 * 
 * Lưu ý:
 * - Phải gọi hàm này trước khi sử dụng LCD
 * - Chỉ gọi 1 lần khi khởi động hệ thống
 * - Tổng thời gian khởi tạo: ~30ms
 * 
 * Mối liên hệ:
 * - Được gọi 1 lần trong main() khi khởi động
 * ============================================================================
 */
void LCD_Init(void)
{
    // Chờ > 15ms sau khi bật nguồn (LCD cần thời gian ổn định)
    delay_ms(20);
    
    // Gửi lệnh Function Set (0x38) lần 1
    // 0x38 = 0b00111000: 8-bit, 2 lines, 5×8 font
    LCD_WriteCommand(0x38);
    delay_ms(5);  // Chờ > 4.1ms
    
    // Gửi lệnh Function Set (0x38) lần 2
    LCD_WriteCommand(0x38);
    delay_us(100);  // Chờ > 100µs
    
    // Gửi lệnh Function Set (0x38) lần 3
    // Sau 3 lần gửi, LCD chắc chắn ở chế độ 8-bit
    LCD_WriteCommand(0x38);
    
    // Gửi lệnh Display Control (0x0C)
    // 0x0C = 0b00001100: Display ON, Cursor OFF, Blink OFF
    LCD_WriteCommand(0x0C);
    
    // Gửi lệnh Entry Mode Set (0x06)
    // 0x06 = 0b00000110: Increment cursor, No shift display
    LCD_WriteCommand(0x06);
    
    // Gửi lệnh Clear Display (0x01)
    // Xóa toàn bộ màn hình và đưa con trỏ về (0,0)
    LCD_WriteCommand(0x01);
    
    // Chờ lệnh Clear Display hoàn tất (cần 1.52ms)
    delay_ms(2);
}

/*
 * ============================================================================
 * HÀM: LCD_SetCursor
 * ============================================================================
 * Mô tả: Di chuyển con trỏ đến vị trí (row, col)
 * Tham số:
 * - row: Số dòng (0 hoặc 1)
 * - col: Số cột (0-15)
 * 
 * DDRAM Address của LCD 16x2:
 * - Dòng 0: 0x00-0x0F (địa chỉ 0x80-0x8F)
 * - Dòng 1: 0x40-0x4F (địa chỉ 0xC0-0xCF)
 * 
 * Lệnh Set DDRAM Address:
 * - Bit 7: Luôn = 1 (để phân biệt với lệnh khác)
 * - Bit 6-0: Địa chỉ DDRAM (0x00-0x7F)
 * 
 * Ví dụ:
 * - (0,0): 0x80 + 0 = 0x80 (đầu dòng 0)
 * - (0,5): 0x80 + 5 = 0x85 (dòng 0, cột 5)
 * - (1,0): 0xC0 + 0 = 0xC0 (đầu dòng 1)
 * - (1,10): 0xC0 + 10 = 0xCA (dòng 1, cột 10)
 * 
 * Ví dụ sử dụng:
 * LCD_SetCursor(0, 0);   // Di chuyển đến (0,0)
 * LCD_Print("Hello");    // Hiển thị "Hello" từ (0,0)
 * LCD_SetCursor(1, 5);   // Di chuyển đến (1,5)
 * LCD_Print("World");    // Hiển thị "World" từ (1,5)
 * 
 * Lưu ý:
 * - row chỉ nhận 0 hoặc 1 (LCD có 2 dòng)
 * - col nhận 0-15 (LCD có 16 cột)
 * - Nếu col > 15, ký tự sẽ không hiển thị
 * 
 * Mối liên hệ:
 * - Được gọi bởi display.c trước khi hiển thị mỗi dòng
 * - Được gọi bởi main.c để hiển thị màn hình chào
 * ============================================================================
 */
void LCD_SetCursor(unsigned char row, unsigned char col)
{
    // Tính địa chỉ DDRAM dựa trên row
    // Dòng 0: 0x80 (0b10000000)
    // Dòng 1: 0xC0 (0b11000000)
    unsigned char address = (row == 0) ? 0x80 : 0xC0;
    
    // Cộng thêm col để có địa chỉ chính xác
    // Ví dụ: row=1, col=5 -> 0xC0 + 5 = 0xC5
    LCD_WriteCommand(address + col);
}

/*
 * ============================================================================
 * HÀM: LCD_Print
 * ============================================================================
 * Mô tả: Hiển thị một chuỗi ký tự lên LCD
 * Tham số: str - Con trỏ đến chuỗi (kết thúc bằng '\0')
 * 
 * Chi tiết hoạt động:
 * 1. Duyệt qua từng ký tự trong chuỗi
 * 2. Gọi LCD_WriteData() để hiển thị từng ký tự
 * 3. LCD tự động tăng con trỏ sau mỗi ký tự
 * 4. Dừng khi gặp '\0' (null terminator)
 * 
 * Ví dụ sử dụng:
 * LCD_SetCursor(0, 0);
 * LCD_Print("Temperature");  // Hiển thị "Temperature" từ (0,0)
 * 
 * LCD_SetCursor(1, 0);
 * LCD_Print("25.6C");  // Hiển thị "25.6C" từ (1,0)
 * 
 * Lưu ý:
 * - Phải gọi LCD_SetCursor() trước để chọn vị trí
 * - Nếu chuỗi dài > 16 ký tự, phần thừa sẽ tràn sang dòng tiếp theo
 * - Không tự động xuống dòng, phải gọi LCD_SetCursor() để chuyển dòng
 * 
 * Mối liên hệ:
 * - Được gọi bởi display.c để hiển thị dữ liệu cảm biến
 * - Được gọi bởi main.c để hiển thị màn hình chào
 * ============================================================================
 */
void LCD_Print(char *str)
{
    // Vòng lặp duyệt qua từng ký tự trong chuỗi
    // Điều kiện dừng: gặp ký tự '\0' (null terminator)
    while (*str)
    {
        // Hiển thị ký tự hiện tại lên LCD
        LCD_WriteData(*str++);
        // *str++: lấy giá trị tại con trỏ, sau đó tăng con trỏ lên 1
    }
}
