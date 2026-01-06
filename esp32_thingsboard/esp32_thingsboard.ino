#if ARDUINO_USB_CDC_ON_BOOT != 1
#warning "USB CDC On Boot should be enabled! Go to Tools -> USB CDC On Boot -> Enabled"
#endif

#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>

#ifdef ARDUINO_USB_MODE
#undef ARDUINO_USB_MODE
#endif
#define ARDUINO_USB_MODE 1

const char *WIFI_SSID     = "Vui Vẻ";
const char *WIFI_PASSWORD = "duyloc0203197";
const char *TB_SERVER     = "thingsboard.cloud";
const char *TB_TOKEN      = "nsxOWH3lpXXnZQJHWwhu";

#define UART_BUFFER_SIZE 80
char uartBuffer[UART_BUFFER_SIZE];
uint8_t uartBufferIndex = 0;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient);

float temperature = 0.0;
float humidity    = 0.0;
float pm25        = 0.0;
unsigned int gas  = 0;

bool err_aht10 = false;
bool err_pm25 = false;
bool err_gas = false;

bool dataReady              = false;
unsigned long lastReconnect = 0;
unsigned long lastSend      = 0;
const unsigned long RECONNECT_INTERVAL = 5000;
const unsigned long SEND_INTERVAL      = 2000;

void WiFi_Setup(const char *ssid, const char *password)
{
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi FAILED!");
    }
}

bool WiFi_IsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String AirQuality_GetStatus(float pm25_value)
{
    if (pm25_value <= 10) return "Rat tot";
    else if (pm25_value <= 25) return "Tot";
    else if (pm25_value <= 50) return "Trung binh";
    else if (pm25_value <= 70) return "Kem";
    else if (pm25_value <= 150) return "Xau";
    else return "Nguy hai";
}

int AirQuality_GetLevel(float pm25_value)
{
    if (pm25_value <= 10) return 1;
    else if (pm25_value <= 25) return 2;
    else if (pm25_value <= 50) return 3;
    else if (pm25_value <= 70) return 4;
    else if (pm25_value <= 150) return 5;
    else return 6;
}

String AirQuality_GetColor(float pm25_value)
{
    if (pm25_value <= 10) return "#0066CC";
    else if (pm25_value <= 25) return "#00AA00";
    else if (pm25_value <= 50) return "#DAA520";
    else if (pm25_value <= 70) return "#FF6600";
    else if (pm25_value <= 150) return "#CC0000";
    else return "#800080";
}

String GasQuality_GetStatus(unsigned int gas_ppm)
{
    if (gas_ppm <= 200) return "An toan";
    else if (gas_ppm <= 400) return "Canh bao";
    else if (gas_ppm <= 700) return "Nguy hiem";
    else return "Rat nguy hiem";
}

String GasQuality_GetColor(unsigned int gas_ppm)
{
    if (gas_ppm <= 200) return "#00AA00";
    else if (gas_ppm <= 400) return "#DAA520";
    else if (gas_ppm <= 700) return "#FF6600";
    else return "#CC0000";
}

bool TB_Connect(const char *server, const char *token)
{
    Serial.print("[TB] Server: ");
    Serial.println(server);
    Serial.print("[TB] Token: ");
    Serial.println(token);
    
    if (tb.connect(server, token, 1883)) {
        Serial.println("[TB] Connected!");
        return true;
    }
    
    Serial.println("[TB] Connection failed!");
    return false;
}

void TB_SendAttributes()
{
    const char* json = "{\"deviceType\":\"Smart Room Monitor\",\"model\":\"8051-ESP32\",\"location\":\"Living Room\",\"firmware\":\"v1.0\"}";
    tb.sendAttributeString(json);
    Serial.println("[TB] Attributes sent!");
}

void TB_SendTelemetry(float temp, float humid, float pm, unsigned int g)
{
    String airQuality = AirQuality_GetStatus(pm);
    String airColor = AirQuality_GetColor(pm);
    int airLevel = AirQuality_GetLevel(pm);
    
    String gasStatus = GasQuality_GetStatus(g);
    String gasColor = GasQuality_GetColor(g);
    
    if (!err_aht10) {
        tb.sendTelemetryData("temperature", temp);
        tb.sendTelemetryData("humidity", humid);
    }
    if (!err_pm25) {
        tb.sendTelemetryData("pm25", pm);
        tb.sendTelemetryData("airQualityStatus", airQuality.c_str());
        tb.sendTelemetryData("airQualityLevel", airLevel);
        tb.sendTelemetryData("airQualityColor", airColor.c_str());
    }
    if (!err_gas) {
        tb.sendTelemetryData("gas", g);
        tb.sendTelemetryData("gasStatus", gasStatus.c_str());
        tb.sendTelemetryData("gasColor", gasColor.c_str());
    }
    
    if (!err_pm25) tb.sendTelemetryData("pm25Alert", pm > 70);
    if (!err_gas) tb.sendTelemetryData("gasAlert", g > 400);
    if (!err_aht10) {
        tb.sendTelemetryData("tempAlert", temp > 35 || temp < 10);
        tb.sendTelemetryData("humidAlert", humid > 80 || humid < 30);
    }
    
    tb.sendTelemetryData("sensorStatus_TempHumid", err_aht10 ? "ERR" : "OK");
    tb.sendTelemetryData("sensorStatus_PM25", err_pm25 ? "ERR" : "OK");
    tb.sendTelemetryData("sensorStatus_Gas", err_gas ? "ERR" : "OK");
    
    String systemStatus = (err_aht10 || err_pm25 || err_gas) ? "ERROR" : "OK";
    tb.sendTelemetryData("systemStatus", systemStatus.c_str());
    
    tb.sendTelemetryData("sensorError_TempHumid", err_aht10);
    tb.sendTelemetryData("sensorError_PM25", err_pm25);
    tb.sendTelemetryData("sensorError_Gas", err_gas);
    
    Serial.println("[TB] Telemetry sent!");
    Serial.printf("  T=%.1f H=%.1f PM=%.0f G=%u ppm\n", temp, humid, pm, g);
    if (err_aht10 || err_pm25 || err_gas) {
        Serial.printf("  [SENSOR ERRORS: T/H=%d PM=%d Gas=%d]\n", err_aht10, err_pm25, err_gas);
    }
}

bool ProcessUARTChar(char c)
{
    if (c == '{' && uartBufferIndex == 0) {
        uartBuffer[uartBufferIndex++] = c;
    }
    else if (c == '\n' && uartBufferIndex > 0) {
        uartBuffer[uartBufferIndex] = '\0';

        if (uartBuffer[0] == '{') {
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, uartBuffer);

            if (!error) {
                temperature = doc["T"] | 0.0f;
                humidity    = doc["H"] | 0.0f;
                pm25        = doc["P"] | 0.0f;
                gas         = doc["G"] | 0;
                
                int err = doc["E"] | 0;
                err_aht10 = (err & 1) ? true : false;
                err_pm25 = (err & 2) ? true : false;
                err_gas = (err & 4) ? true : false;

                Serial.printf("Data: T=%.1f H=%.1f PM=%.0f G=%u", 
                              temperature, humidity, pm25, gas);
                if (err > 0) {
                    Serial.printf(" [ERR=%d: T=%d P=%d G=%d]", err, err_aht10, err_pm25, err_gas);
                }
                Serial.println();
                uartBufferIndex = 0;
                return true;
            } else {
                Serial.print("[UART] JSON error: ");
                Serial.println(error.c_str());
            }
        }
        uartBufferIndex = 0;
    }
    else if (uartBufferIndex > 0 && uartBufferIndex < UART_BUFFER_SIZE - 1) {
        uartBuffer[uartBufferIndex++] = c;
    }
    else if (uartBufferIndex >= UART_BUFFER_SIZE - 1) {
        Serial.println("[UART] Buffer overflow");
        uartBufferIndex = 0;
    }
    return false;
}

void setup()
{
    Serial.begin(115200);
    
    unsigned long start = millis();
    while (!Serial && (millis() - start < 3000)) {
        delay(10);
    }
    
    delay(500);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32-S2 MINI SENSOR MONITOR");
    Serial.println("========================================");

    Serial1.begin(9600, SERIAL_8N1, 9, 8);
    Serial.println("[INFO] UART: GPIO9=RX, GPIO8=TX");
    
    WiFi_Setup(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.println("System Ready!\n");
}

void loop()
{
    if (!Serial) {
        Serial.begin(115200);
    }
    
    if (!WiFi_IsConnected()) {
        Serial.println("[WiFi] Reconnecting...");
        WiFi_Setup(WIFI_SSID, WIFI_PASSWORD);
    }

    if (!tb.connected()) {
        if (millis() - lastReconnect > RECONNECT_INTERVAL) {
            lastReconnect = millis();
            Serial.println("[TB] Connecting...");
            if (TB_Connect(TB_SERVER, TB_TOKEN)) {
                TB_SendAttributes();
            }
        }
    }
    tb.loop();

    while (Serial1.available()) {
        char c = Serial1.read();
        if (ProcessUARTChar(c)) {
            dataReady = true;
        }
    }

    if (dataReady && (millis() - lastSend > SEND_INTERVAL)) {
        lastSend = millis();
        
        if (tb.connected()) {
            TB_SendTelemetry(temperature, humidity, pm25, gas);
        }
        
        Serial.println("=== SENSOR DATA ===");
        Serial.printf("Temperature: %.1f C\n", temperature);
        Serial.printf("Humidity: %.1f %%\n", humidity);
        Serial.printf("PM2.5: %.0f ug/m3\n", pm25);
        Serial.printf("Gas: %u ppm\n", gas);
        Serial.printf("Air Quality: %s\n", AirQuality_GetStatus(pm25).c_str());
        Serial.println("==================");
        
        dataReady = false;
    }
    
    static unsigned long lastNoDataMsg = 0;
    if (!dataReady && (millis() - lastNoDataMsg > 10000)) {
        lastNoDataMsg = millis();
        Serial.println("[WARN] No data from 8051 - Check UART connection!");
    }
}
