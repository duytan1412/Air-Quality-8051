# 🌱 Air Quality Monitoring System (8051 & ESP32)

**Advanced IoT Environmental Monitoring with Layered Firmware Architecture**

![8051](https://img.shields.io/badge/MCU-8051-green)
![Keil](https://img.shields.io/badge/IDE-Keil%20uVision%205-orange)
![ESP32](https://img.shields.io/badge/SoC-ESP32-blue)
![FPT Jetking](https://img.shields.io/badge/FPT%20Jetking-Featured%20Project-blue)

---

## 📋 Description | Mô tả

**🇬🇧 English:**  
A complex **IoT System** for real-time monitoring of PM2.5, Gas, Temperature, and Humidity. This project showcases a professional **Layered Architecture** (Drivers -> Core -> Application) on a resource-constrained 8051 MCU, with data bridging to **ThingsBoard Cloud** via ESP32.

**🇻🇳 Tiếng Việt:**  
Hệ thống **IoT phức hợp** giám sát thời gian thực bụi mịn PM2.5, Khí gas, Nhiệt độ và Độ ẩm. Dự án thể hiện **Kiến trúc phân lớp chuyên nghiệp** (Drivers -> Core -> Application) trên vi điều khiển 8051 hạn chế tài nguyên, kết nối dữ liệu lên **ThingsBoard Cloud** thông qua ESP32.

---

## 🛠️ Hardware Deep Dive | Chi tiết phần cứng

| Component | Technical Details | Role |
|-----------|------------------|------|
| **ADS1115** | 16-bit High Precision ADC via I2C | High-res data for Analog sensors |
| **GP2Y1014** | Optical Dust Sensor (PM2.5) | Dust density measurement |
| **MQ-2** | Gas/Smoke Sensor | Air quality & Safety alert |
| **AHT10** | Digital Temp & Humid (I2C) | Precise environment sensing |
| **LCD 16x2** | Parallel interface (8-bit) | Local UI monitoring |
| **ESP32** | SoC with Wi-Fi | Gateway to ThingsBoard Cloud |

---

## 🔧 Software Architecture | Kiến trúc phần mềm

### 📁 1. Drivers Layer (`drivers/`)
- **`i2c.c/h`**: Software I2C implementation for ADS1115 and AHT10.
- **`ads1115.c/h`**: 16-bit ADC driver handling gain and conversion.
- **`lcd.c/h`**: Driver for character LCD 16x2.
- **`mq2.c/h` & `gp2y1014.c/h`**: Sensor signal processing & calibration.

### 📁 2. Core Layer (`core/`)
- **`uart_protocol.c/h`**: Custom communication protocol between 8051 and ESP32.
- **`watchdog.c/h`**: System reliability and auto-recovery.
- **`display.c/h`**: High-level UI management for the LCD.

### 📁 3. Cloud Integration (`esp32_thingsboard/`)
- ESP32 acts as a **smart gateway**, receiving UART packets from 8051, parsing them into JSON, and publishing to **ThingsBoard** via MQTT.

---

## 📡 UART Communication Protocol | Giao thức truyền thông

**Packet Format:** `[START] [PM2.5] [GAS] [TEMP] [HUMID] [CHECKSUM] [END]`
- **Baudrate:** 9600 bps
- **Sync:** Asynchronous interrupt-based reception on 8051.

---

## 🏆 Technical Highlights | Điểm nhấn kỹ thuật

**🇬🇧 English:**
- **Layered Design:** Decoupling hardware drivers from business logic for portability.
- **Interrupt Management:** Critical for handling real-time sensor sampling and UART tasks.
- **Resource Optimization:** Running a complex system on 128 bytes of DATA RAM.
- **Cloud Connectivity:** Real-time data visualization on a professional IoT Dashboard.

**🇻🇳 Tiếng Việt:**
- **Thiết kế phân lớp:** Tách rời driver phần cứng khỏi logic nghiệp vụ giúp dễ bảo trì và mở rộng.
- **Quản lý ngắt:** Rất quan trọng để xử lý việc lấy mẫu cảm biến và truyền UART thời gian thực.
- **Tối ưu hóa tài nguyên:** Chạy một hệ thống phức tạp chỉ với 128 bytes RAM.
- **Kết nối Cloud:** Trực quan hóa dữ liệu thời gian thực trên Dashboard IoT chuyên nghiệp.

---

## 👨‍💻 Author | Tác giả

**Bì Duy Tân**
- 🎓 FPT Jetking (Featured Student Project)
- 🎯 Embedded Firmware / IoT Developer
- 📧 duytan2903@gmail.com
- 🔗 [Featured on FPT Jetking Page](https://www.facebook.com/share/p/17SBQQEq5k/)
