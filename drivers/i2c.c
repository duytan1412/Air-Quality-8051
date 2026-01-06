/*
 * ============================================================================
 * I2C SOFTWARE IMPLEMENTATION - i2c.c
 * ============================================================================
 * Mô tả: Giao tiếp I2C bằng phần mềm (bit-banging)
 * Lý do: 8051 không có phần cứng I2C tích hợp
 * 
 * Giao thức I2C:
 * - 2 dây: SDA (data), SCL (clock)
 * - Master (8051) điều khiển clock
 * - Slave (AHT10, ADS1115) phản hồi ACK
 * 
 * Tốc độ: ~100kHz (Standard Mode)
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi aht10.c để đọc nhiệt độ/độ ẩm
 * - Được gọi bởi ads1115.c để đọc ADC
 * - Sử dụng delay.c để tạo timing chính xác
 * ============================================================================
 */

#include "i2c.h"
#include "delay.h"

// Biến toàn cục lưu trạng thái lỗi I2C
// 0 = OK, 1 = Timeout (slave không ACK)
unsigned char i2c_error = 0;

/*
 * ============================================================================
 * HÀM: I2C_Init
 * ============================================================================
 * Mô tả: Khởi tạo chân SDA và SCL ở trạng thái IDLE
 * 
 * Trạng thái IDLE của I2C:
 * - SDA = 1 (HIGH)
 * - SCL = 1 (HIGH)
 * 
 * Lưu ý:
 * - Chân SDA và SCL cần điện trở pull-up (4.7kΩ)
 * - Không cần cấu hình thêm vì 8051 port là quasi-bidirectional
 * 
 * Mối liên hệ:
 * - Được gọi 1 lần trong main() khi khởi động
 * - Phải gọi trước khi sử dụng bất kỳ hàm I2C nào
 * ============================================================================
 */
void I2C_Init(void)
{
    // Đặt SDA = 1 (HIGH) - trạng thái IDLE
    SDA = 1;
    
    // Đặt SCL = 1 (HIGH) - trạng thái IDLE
    SCL = 1;
}

/*
 * ============================================================================
 * HÀM: I2C_Start
 * ============================================================================
 * Mô tả: Tạo điều kiện START của giao thức I2C
 * 
 * Điều kiện START:
 * - SDA chuyển từ HIGH -> LOW trong khi SCL = HIGH
 * 
 * Timing:
 *     SDA: ‾‾‾‾‾‾‾‾‾‾\_________
 *     SCL: ‾‾‾‾‾‾‾‾‾‾‾‾‾‾\_____
 *          ^         ^
 *          |         |
 *        IDLE     START
 * 
 * Ý nghĩa:
 * - Báo hiệu cho tất cả slave: "Master bắt đầu truyền"
 * - Sau START, master sẽ gửi địa chỉ slave
 * 
 * Mối liên hệ:
 * - Được gọi trước mỗi lần truyền I2C
 * - Sau START phải gọi I2C_WriteByte() để gửi địa chỉ slave
 * ============================================================================
 */
void I2C_Start(void)
{
    // Đảm bảo SDA và SCL ở trạng thái IDLE (HIGH)
    SDA = 1;
    SCL = 1;
    delay_us(5);  // Chờ ổn định
    
    // Kéo SDA xuống LOW (trong khi SCL vẫn HIGH)
    // Đây là điều kiện START
    SDA = 0;
    delay_us(5);  // Hold time cho START condition
    
    // Kéo SCL xuống LOW để chuẩn bị truyền dữ liệu
    SCL = 0;
}

/*
 * ============================================================================
 * HÀM: I2C_Stop
 * ============================================================================
 * Mô tả: Tạo điều kiện STOP của giao thức I2C
 * 
 * Điều kiện STOP:
 * - SDA chuyển từ LOW -> HIGH trong khi SCL = HIGH
 * 
 * Timing:
 *     SDA: _________/‾‾‾‾‾‾‾‾‾‾
 *     SCL: _____/‾‾‾‾‾‾‾‾‾‾‾‾‾
 *               ^   ^
 *               |   |
 *             SCL  STOP
 *            HIGH
 * 
 * Ý nghĩa:
 * - Báo hiệu cho tất cả slave: "Master kết thúc truyền"
 * - Bus I2C trở về trạng thái IDLE
 * 
 * Mối liên hệ:
 * - Được gọi sau khi hoàn tất truyền/nhận dữ liệu
 * - Sau STOP, bus I2C sẵn sàng cho lần truyền tiếp theo
 * ============================================================================
 */
void I2C_Stop(void)
{
    // Đảm bảo SDA = LOW trước khi tạo STOP
    SDA = 0;
    
    // Kéo SCL lên HIGH
    SCL = 1;
    delay_us(5);  // Setup time cho STOP condition
    
    // Kéo SDA lên HIGH (trong khi SCL = HIGH)
    // Đây là điều kiện STOP
    SDA = 1;
    delay_us(5);  // Hold time cho STOP condition
}

/*
 * ============================================================================
 * HÀM: I2C_WriteByte
 * ============================================================================
 * Mô tả: Gửi 1 byte dữ liệu qua I2C và nhận ACK từ slave
 * Tham số: dat - byte cần gửi (8 bit)
 * Trả về: ACK bit (0=ACK, 1=NACK)
 * 
 * Quy trình gửi 1 byte:
 * 1. Gửi 8 bit dữ liệu (MSB first)
 * 2. Nhận 1 bit ACK từ slave
 * 
 * Timing cho mỗi bit:
 *     SDA: ___/‾‾‾\___  (dữ liệu)
 *     SCL: ___/‾‾‾\___  (clock)
 *          ^   ^   ^
 *          |   |   |
 *        Setup Sample Hold
 * 
 * ACK bit:
 * - Slave kéo SDA = 0 để xác nhận nhận được dữ liệu
 * - Nếu SDA = 1 (NACK), có thể slave không tồn tại hoặc bận
 * 
 * Mối liên hệ:
 * - Được gọi sau I2C_Start() để gửi địa chỉ slave
 * - Được gọi để gửi lệnh và dữ liệu đến slave
 * - Kiểm tra ACK để phát hiện lỗi giao tiếp
 * ============================================================================
 */
bit I2C_WriteByte(unsigned char dat)
{
    unsigned char i;
    unsigned char timeout;
    bit ack;

    // ========================================================================
    // BƯỚC 1: GỬI 8 BIT DỮ LIỆU (MSB FIRST)
    // ========================================================================
    for (i = 0; i < 8; i++)
    {
        // Đặt SDA = bit cao nhất của dat (bit 7)
        // Dùng phép AND với 0x80 (0b10000000) để lấy bit 7
        SDA = (dat & 0x80) ? 1 : 0;
        delay_us(2);  // Setup time: SDA phải ổn định trước khi SCL lên HIGH
        
        // Tạo xung clock: SCL lên HIGH
        // Slave sẽ đọc SDA khi SCL = HIGH
        SCL = 1;
        delay_us(5);  // Clock HIGH time
        
        // Kéo SCL xuống LOW để chuẩn bị bit tiếp theo
        SCL = 0;
        
        // Dịch trái dat 1 bit để chuẩn bị bit tiếp theo
        // Ví dụ: 0b10110010 -> 0b01100100
        dat <<= 1;
    }

    // ========================================================================
    // BƯỚC 2: NHẬN ACK BIT TỪ SLAVE
    // ========================================================================
    // Thả SDA (cho phép slave điều khiển SDA)
    SDA = 1;
    delay_us(2);  // Setup time
    
    // Tạo xung clock cho ACK bit
    SCL = 1;

    // Chờ slave kéo SDA xuống LOW (ACK)
    // Timeout sau I2C_TIMEOUT lần lặp (~200 lần)
    timeout = I2C_TIMEOUT;
    while (SDA && --timeout)
    {
        // Vòng lặp chờ: nếu SDA = 0 (ACK) thì thoát
        // Nếu timeout = 0 thì slave không phản hồi
    }

    // Đọc ACK bit: 0 = ACK (OK), 1 = NACK (lỗi)
    ack = SDA;
    delay_us(5);  // Clock HIGH time
    
    // Kéo SCL xuống LOW
    SCL = 0;

    // Kiểm tra timeout
    if (timeout == 0)
    {
        // Slave không phản hồi -> đánh dấu lỗi
        i2c_error = 1;
    }

    // Trả về ACK bit (0=thành công, 1=lỗi)
    return ack;
}

/*
 * ============================================================================
 * HÀM: I2C_ReadByte
 * ============================================================================
 * Mô tả: Nhận 1 byte dữ liệu từ slave qua I2C
 * Tham số: ack - ACK bit gửi cho slave (0=ACK, 1=NACK)
 * Trả về: Byte dữ liệu nhận được (8 bit)
 * 
 * Quy trình nhận 1 byte:
 * 1. Nhận 8 bit dữ liệu từ slave (MSB first)
 * 2. Gửi ACK/NACK cho slave
 * 
 * ACK/NACK:
 * - ACK (0): Master muốn nhận thêm byte
 * - NACK (1): Master kết thúc nhận (byte cuối cùng)
 * 
 * Timing cho mỗi bit:
 *     SDA: ___/‾‾‾\___  (dữ liệu từ slave)
 *     SCL: ___/‾‾‾\___  (clock từ master)
 *              ^
 *              |
 *           Sample
 * 
 * Mối liên hệ:
 * - Được gọi sau I2C_WriteByte() để đọc dữ liệu từ slave
 * - Gửi ACK=0 nếu muốn đọc thêm byte
 * - Gửi NACK=1 trước I2C_Stop() (byte cuối cùng)
 * ============================================================================
 */
unsigned char I2C_ReadByte(bit ack)
{
    unsigned char i, dat = 0;

    // Thả SDA (cho phép slave điều khiển SDA)
    SDA = 1;
    
    // ========================================================================
    // BƯỚC 1: NHẬN 8 BIT DỮ LIỆU (MSB FIRST)
    // ========================================================================
    for (i = 0; i < 8; i++)
    {
        delay_us(2);  // Setup time
        
        // Tạo xung clock: SCL lên HIGH
        // Slave sẽ đặt dữ liệu lên SDA
        SCL = 1;
        
        // Dịch trái dat 1 bit để chuẩn bị nhận bit mới
        // Ví dụ: 0b00000001 -> 0b00000010
        dat <<= 1;
        
        // Đọc SDA (bit từ slave)
        if (SDA)
        {
            // Nếu SDA = 1, set bit 0 của dat = 1
            dat |= 0x01;
        }
        // Nếu SDA = 0, bit 0 của dat giữ nguyên = 0
        
        delay_us(5);  // Clock HIGH time
        
        // Kéo SCL xuống LOW để chuẩn bị bit tiếp theo
        SCL = 0;
    }

    // ========================================================================
    // BƯỚC 2: GỬI ACK/NACK CHO SLAVE
    // ========================================================================
    // Đặt SDA = ACK bit (0=ACK, 1=NACK)
    SDA = ack ? 1 : 0;
    delay_us(2);  // Setup time
    
    // Tạo xung clock cho ACK bit
    SCL = 1;
    delay_us(5);  // Clock HIGH time
    SCL = 0;
    
    // Thả SDA về HIGH (trạng thái mặc định)
    SDA = 1;

    // Trả về byte dữ liệu nhận được
    return dat;
}
