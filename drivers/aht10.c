/*
 * ============================================================================
 * AHT10 TEMPERATURE & HUMIDITY SENSOR DRIVER - aht10.c
 * ============================================================================
 * Mô tả: Driver cảm biến nhiệt độ và độ ẩm AHT10
 * Giao tiếp: I2C (địa chỉ 0x38)
 * 
 * Thông số cảm biến:
 * - Nhiệt độ: -40°C đến +85°C (độ chính xác ±0.3°C)
 * - Độ ẩm: 0% đến 100% RH (độ chính xác ±2%)
 * - Độ phân giải: 20-bit
 * - Thời gian đo: ~80ms
 * 
 * Nguyên lý hoạt động:
 * 1. Gửi lệnh calibration (0xE1) khi khởi động
 * 2. Gửi lệnh trigger measurement (0xAC)
 * 3. Chờ 80ms để cảm biến đo
 * 4. Đọc 6 bytes dữ liệu qua I2C
 * 5. Tính toán nhiệt độ và độ ẩm từ dữ liệu thô
 * 
 * Format dữ liệu (6 bytes):
 * - Byte 0: Status
 * - Byte 1-2-3[7:4]: Humidity (20-bit)
 * - Byte 3[3:0]-4-5: Temperature (20-bit)
 * 
 * Mối liên hệ với các module khác:
 * - Được gọi bởi main.c mỗi 500ms để đọc nhiệt độ/độ ẩm
 * - Sử dụng i2c.c để giao tiếp với cảm biến
 * - Sử dụng delay.c để chờ cảm biến đo xong
 * ============================================================================
 */

#include "aht10.h"
#include "i2c.h"
#include "delay.h"

/*
 * ============================================================================
 * HÀM: AHT10_Init
 * ============================================================================
 * Mô tả: Khởi tạo cảm biến AHT10
 * 
 * Quy trình khởi tạo:
 * 1. Chờ 40ms sau khi bật nguồn (cảm biến cần thời gian ổn định)
 * 2. Gửi lệnh calibration (0xE1, 0x08, 0x00)
 * 3. Chờ 10ms để calibration hoàn tất
 * 
 * Lệnh calibration (0xE1):
 * - Byte 1: 0xE1 (Initialize command)
 * - Byte 2: 0x08 (Calibration enable)
 * - Byte 3: 0x00 (Reserved)
 * 
 * Lưu ý:
 * - Phải gọi hàm này trước khi đọc dữ liệu
 * - Chỉ gọi 1 lần khi khởi động hệ thống
 * - Nếu không calibration, dữ liệu sẽ không chính xác
 * 
 * Mối liên hệ:
 * - Được gọi 1 lần trong main() khi khởi động
 * - Phải gọi sau I2C_Init()
 * ============================================================================
 */
void AHT10_Init(void)
{
    // Chờ 40ms sau khi bật nguồn
    // Cảm biến cần thời gian để ổn định nguồn và khởi động
    delay_ms(40);

    // Bắt đầu giao tiếp I2C
    I2C_Start();
    
    // Gửi địa chỉ AHT10 (0x38) + bit Write (0)
    // 0x38 << 1 = 0x70 (địa chỉ 7-bit dịch trái, bit 0 = 0 cho Write)
    I2C_WriteByte(AHT10_ADDR << 1);
    
    // Gửi lệnh Initialize (0xE1)
    I2C_WriteByte(0xE1);
    
    // Gửi tham số calibration enable (0x08)
    I2C_WriteByte(0x08);
    
    // Gửi tham số reserved (0x00)
    I2C_WriteByte(0x00);
    
    // Kết thúc giao tiếp I2C
    I2C_Stop();

    // Chờ 10ms để calibration hoàn tất
    delay_ms(10);
}

/*
 * ============================================================================
 * HÀM: AHT10_Read
 * ============================================================================
 * Mô tả: Đọc nhiệt độ và độ ẩm từ cảm biến AHT10
 * 
 * Tham số:
 * - temp_x10: Con trỏ lưu nhiệt độ nhân 10 (ví dụ: 256 = 25.6°C)
 * - humid_x10: Con trỏ lưu độ ẩm nhân 10 (ví dụ: 653 = 65.3%)
 * 
 * Quy trình đọc:
 * 1. Gửi lệnh trigger measurement (0xAC, 0x33, 0x00)
 * 2. Chờ 80ms để cảm biến đo
 * 3. Đọc 6 bytes dữ liệu từ cảm biến
 * 4. Tách dữ liệu humidity (20-bit) và temperature (20-bit)
 * 5. Chuyển đổi sang đơn vị thực (°C và %)
 * 
 * Format dữ liệu 6 bytes:
 * - data0: Status byte (bit 7 = busy)
 * - data1: Humidity[19:12]
 * - data2: Humidity[11:4]
 * - data3[7:4]: Humidity[3:0]
 * - data3[3:0]: Temperature[19:16]
 * - data4: Temperature[15:8]
 * - data5: Temperature[7:0]
 * 
 * Công thức chuyển đổi:
 * - Humidity (%) = (humidity_raw / 2^20) × 100
 * - Temperature (°C) = (temperature_raw / 2^20) × 200 - 50
 * 
 * Lưu ý:
 * - Phải gọi AHT10_Init() trước
 * - Thời gian đo: ~80ms (blocking)
 * - Kết quả nhân 10 để tránh dùng float
 * 
 * Mối liên hệ:
 * - Được gọi bởi main.c mỗi 500ms
 * - Kết quả được kiểm tra lỗi trong main.c
 * - Kết quả được hiển thị lên LCD và gửi qua UART
 * ============================================================================
 */
void AHT10_Read(int *temp_x10, int *humid_x10)
{
    // Biến lưu 6 bytes dữ liệu từ cảm biến
    unsigned char data0, data1, data2, data3, data4, data5;
    
    // Biến lưu giá trị thô 20-bit
    unsigned long humidity_raw;
    unsigned long temperature_raw;

    // ========================================================================
    // BƯỚC 1: GỬI LỆNH TRIGGER MEASUREMENT
    // ========================================================================
    I2C_Start();
    
    // Gửi địa chỉ AHT10 + bit Write
    I2C_WriteByte(AHT10_ADDR << 1);
    
    // Gửi lệnh Trigger Measurement (0xAC)
    I2C_WriteByte(0xAC);
    
    // Gửi tham số (0x33)
    I2C_WriteByte(0x33);
    
    // Gửi tham số (0x00)
    I2C_WriteByte(0x00);
    
    I2C_Stop();

    // ========================================================================
    // BƯỚC 2: CHỜ CẢM BIẾN ĐO XONG
    // ========================================================================
    // Cảm biến cần ~75ms để đo, chờ 80ms để an toàn
    delay_ms(80);

    // ========================================================================
    // BƯỚC 3: ĐỌC 6 BYTES DỮ LIỆU
    // ========================================================================
    I2C_Start();
    
    // Gửi địa chỉ AHT10 + bit Read (1)
    I2C_WriteByte((AHT10_ADDR << 1) | 0x01);
    
    // Đọc 6 bytes dữ liệu
    // ACK (0) cho 5 bytes đầu, NACK (1) cho byte cuối
    data0 = I2C_ReadByte(0);  // Status byte
    data1 = I2C_ReadByte(0);  // Humidity[19:12]
    data2 = I2C_ReadByte(0);  // Humidity[11:4]
    data3 = I2C_ReadByte(0);  // Humidity[3:0] + Temperature[19:16]
    data4 = I2C_ReadByte(0);  // Temperature[15:8]
    data5 = I2C_ReadByte(1);  // Temperature[7:0] - NACK
    
    I2C_Stop();

    // ========================================================================
    // BƯỚC 4: TÁCH DỮ LIỆU HUMIDITY (20-BIT)
    // ========================================================================
    // Humidity = data1[7:0] + data2[7:0] + data3[7:4]
    // Bit 19-12: data1
    // Bit 11-4: data2
    // Bit 3-0: data3[7:4]
    humidity_raw = ((unsigned long)data1 << 12) |
                   ((unsigned long)data2 << 4) |
                   (data3 >> 4);

    // ========================================================================
    // BƯỚC 5: TÁCH DỮ LIỆU TEMPERATURE (20-BIT)
    // ========================================================================
    // Temperature = data3[3:0] + data4[7:0] + data5[7:0]
    // Bit 19-16: data3[3:0]
    // Bit 15-8: data4
    // Bit 7-0: data5
    temperature_raw = (((unsigned long)data3 & 0x0F) << 16) |
                      ((unsigned long)data4 << 8) |
                      data5;

    // ========================================================================
    // BƯỚC 6: CHUYỂN ĐỔI SANG ĐƠN VỊ THỰC
    // ========================================================================
    // Công thức Humidity (%):
    // H = (humidity_raw / 2^20) × 100
    // H × 10 = (humidity_raw × 1000) / 1048576
    // 1048576 = 2^20
    *humid_x10 = (int)((humidity_raw * 1000UL) / 1048576UL);
    
    // Công thức Temperature (°C):
    // T = (temperature_raw / 2^20) × 200 - 50
    // T × 10 = (temperature_raw × 2000) / 1048576 - 500
    *temp_x10 = (int)((temperature_raw * 2000UL) / 1048576UL) - 500;
}
