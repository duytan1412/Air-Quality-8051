# 🌱 Air Quality Monitoring System (8051)

**IoT Environmental Monitoring with 8051 Microcontroller**

![8051](https://img.shields.io/badge/MCU-8051-green)
![Keil](https://img.shields.io/badge/IDE-Keil%20uVision%205-orange)
![FPT Jetking](https://img.shields.io/badge/FPT%20Jetking-Featured%20Project-blue)

---

## 📋 Description | Mô tả

**🇬🇧 English:**  
A real-time **IoT Air Quality Monitoring System** using 8051 microcontroller. This project demonstrates embedded firmware development skills including register-level programming, interrupt handling, and multi-MCU communication via UART.

> 🏆 **Featured Project** on [FPT Jetking Official Fanpage](https://www.facebook.com/share/p/17SBQQEq5k/)

**🇻🇳 Tiếng Việt:**  
Hệ thống **Giám sát Chất lượng Không khí IoT** thời gian thực sử dụng vi điều khiển 8051. Dự án thể hiện kỹ năng phát triển firmware nhúng bao gồm lập trình cấp thanh ghi, xử lý ngắt và giao tiếp đa vi điều khiển qua UART.

> 🏆 **Dự án tiêu biểu** được đăng trên [Fanpage chính thức FPT Jetking](https://www.facebook.com/share/p/17SBQQEq5k/)

---

## ✨ Key Features | Tính năng chính

| Feature | Description | Mô tả |
|---------|-------------|-------|
| **Real-time Monitoring** | Continuous air quality measurement | Đo chất lượng không khí liên tục |
| **LCD Display** | Visual data output on screen | Hiển thị dữ liệu trực quan trên màn hình |
| **Auto Alert** | Warning when threshold exceeded | Cảnh báo khi vượt ngưỡng cho phép |
| **Low Power** | Optimized for 24/7 operation | Tối ưu hoạt động 24/7 |

---

## 🛠️ Technology Stack | Công nghệ sử dụng

| Component | Details | Chi tiết |
|-----------|---------|----------|
| **MCU** | 8051 Microcontroller | Vi điều khiển 8051 |
| **IDE** | Keil uVision 5 | Keil uVision 5 |
| **Language** | Embedded C / Assembly | C nhúng / Assembly |
| **Communication** | UART (8051 ↔ ESP32) | UART (8051 ↔ ESP32) |
| **Sensors** | Air quality sensors | Cảm biến chất lượng không khí |

---

## 🔧 Technical Highlights | Điểm kỹ thuật nổi bật

**🇬🇧 English:**
- **Register-level programming** - Direct manipulation of 8051 SFRs (Special Function Registers)
- **Interrupt handling** - Timer and UART interrupts for responsive system
- **UART communication** - Asynchronous data transfer between 8051 and ESP32
- **Memory optimization** - Efficient code for limited 8051 resources (128 bytes RAM)

**🇻🇳 Tiếng Việt:**
- **Lập trình cấp thanh ghi** - Thao tác trực tiếp với SFRs của 8051
- **Xử lý ngắt** - Ngắt Timer và UART cho hệ thống đáp ứng nhanh
- **Giao tiếp UART** - Truyền dữ liệu bất đồng bộ giữa 8051 và ESP32
- **Tối ưu bộ nhớ** - Code hiệu quả cho tài nguyên hạn chế của 8051 (128 bytes RAM)

---

## 📁 Project Structure | Cấu trúc dự án

```
Air-Quality-8051/
├── main.c              # Main program / Chương trình chính
├── *.uvproj            # Keil project file
├── README.md           # This file
├── .gitignore          # Ignore build files
└── ...
```

---

## 🚀 How to Run | Cách chạy

**🇬🇧 English:**
1. Open Keil uVision 5
2. Open project file (`.uvproj`)
3. Build project (F7)
4. Flash to 8051 board

**🇻🇳 Tiếng Việt:**
1. Mở Keil uVision 5
2. Mở file project (`.uvproj`)
3. Build project (F7)
4. Nạp vào board 8051

---

## 🎓 What I Learned | Những gì tôi học được

**🇬🇧 English:**
- Low-level embedded programming on resource-constrained MCU
- Hardware debugging and timing analysis
- Multi-MCU system design with UART protocol
- Sensor data processing and calibration

**🇻🇳 Tiếng Việt:**
- Lập trình nhúng cấp thấp trên MCU tài nguyên hạn chế
- Debug phần cứng và phân tích timing
- Thiết kế hệ thống đa MCU với giao thức UART
- Xử lý và hiệu chỉnh dữ liệu cảm biến

---

## 👨‍💻 Author | Tác giả

**Bì Duy Tân**
- 🎓 FPT Jetking - Chip Design Technology (Semester 1)
- 🎯 Target: Embedded Tester @ FPT Software Automotive  
- 📧 duytan2903@gmail.com
- 🔗 [LinkedIn](https://www.linkedin.com/in/duy-t%C3%A2n-b-439ba0153/)
- 💻 [GitHub](https://github.com/duytan1412)

---

## 📝 License

MIT License - Free to use for learning purposes.
