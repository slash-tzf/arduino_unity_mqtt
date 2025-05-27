#include <WiFiS3.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TinyGPSPlus.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

// 宏定义
#define LOG_PERIOD 15000  // 日志记录周期(毫秒)，推荐值15000-60000
#define MAX_PERIOD 60000  // 最大日志周期
#define ONE_WIRE_BUS 4    // 温度传感器数据引脚
#define BH1750_ADDR 0x23  // BH1750光照传感器地址

// ===== WiFi和MQTT配置 =====
// WiFi配置
const char ssid[] = "Tenda_E39790";
const char pass[] = "gao707707";

// MQTT配置
const char mqtt_user[] = "";  // MQTT用户名
const char mqtt_pass[] = "";  // MQTT密码
const char broker[] = "10.103.238.204"; // MQTT服务器地址 116.62.62.70 公网ip
const int port = 1883;  // MQTT端口
const char publish_topic[] = "unity/test";  // 发布主题

// ===== 传感器变量 =====
// GPS相关变量
TinyGPSPlus gps;
double gpsLat = 0.0;  // GPS纬度
double gpsLng = 0.0;  // GPS经度
float gpsAlt = 0.0;   // GPS高度
int gpsSats = 0;      // 可见卫星数量

// 温度传感器相关变量
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// 光照传感器相关变量
byte buff[2];
uint16_t val = 0;

// 辐射传感器相关变量
unsigned long counts = 0;      // GM管事件计数
unsigned long cpm = 0;         // 每分钟计数
unsigned int multiplier;       // 计算CPM的乘数
unsigned long previousMillis;  // 时间测量变量

// ===== 通信对象 =====
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// 函数前向声明
void connectToWifi();
void connectToMqtt();

// ===== 传感器函数 =====
/**
 * GM管事件计数回调函数
 */
void tube_impulse() {
  counts++;
}

/**
 * 读取BH1750光照传感器数据
 * @param address 传感器地址
 * @return 读取的字节数
 */
int BH1750_Read(int address) {
  int i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while (Wire.available()) {
    buff[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();
  return i;
}
 
/**
 * 初始化BH1750光照传感器
 * @param address 传感器地址
 */
void BH1750_Init(int address) {
  Wire.beginTransmission(address);
  Wire.write(0x10); // 设置为高分辨率模式
  Wire.endTransmission();
}

/**
 * 发布消息到MQTT服务器
 * @param buffer 要发送的消息
 */
void publishMessage(char* buffer) {
  // 检查MQTT客户端是否已连接
  if (!mqttClient.connected()) {
    Serial.println("MQTT not connected, cannot publish message");
    return;  // 如果未连接，直接返回不发送消息
  }
  
  // 创建一个JSON对象
  StaticJsonDocument<200> doc;
  doc["msg"] = buffer;
  
  // 将JSON对象序列化为字符串
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  
  // 向指定的主题发送JSON字符串
  mqttClient.beginMessage(publish_topic);
  mqttClient.print(jsonBuffer);
  mqttClient.endMessage();
  
  // 每隔一段时间发送一次
  delay(5000);
}

/**
 * 初始化设置
 */
void setup() {
  // 初始化传感器变量
  counts = 0;
  cpm = 0;
  multiplier = MAX_PERIOD / LOG_PERIOD;
  
  // 初始化通信
  Wire.begin();
  BH1750_Init(BH1750_ADDR);
  Serial.begin(9600);
  Serial1.begin(38400);  // GPS模块波特率，根据实际情况调整
  
  // 初始化其他传感器
  sensors.begin();
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), tube_impulse, FALLING);
  
  // 等待串口准备就绪(最多等待5秒)
  unsigned long startTime = millis();
  while (!Serial && millis() - startTime < 5000) {
    delay(10);
  }
  
  // 连接WiFi网络
  connectToWifi();
  
  // 连接MQTT服务器
  connectToMqtt();
}

/**
 * 连接到WiFi网络
 */
void connectToWifi() {
  Serial.print("正在连接WiFi网络: ");
  Serial.println(ssid);
  
  int wifiRetries = 0;
  while (WiFi.begin(ssid, pass) != WL_CONNECTED && wifiRetries < 10) {
    Serial.print(".");
    wifiRetries++;
    delay(3000);
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("多次尝试后WiFi连接失败，继续执行...");
  } else {
    Serial.println("WiFi连接成功!");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
  }
}

/**
 * 连接到MQTT服务器
 */
void connectToMqtt() {
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);
  Serial.print("正在连接MQTT服务器: ");
  Serial.println(broker);
  
  int mqttRetries = 0;
  while (!mqttClient.connect(broker, port) && mqttRetries < 5) {
    Serial.print("MQTT连接失败! 错误代码 = ");
    Serial.println(mqttClient.connectError());
    Serial.println("2秒后重试...");
    mqttRetries++;
    delay(2000);
  }
  
  if (mqttClient.connected()) {
    Serial.println("MQTT服务器连接成功!");
  } else {
    Serial.println("多次尝试后MQTT连接失败，将继续执行但不发送数据...");
  }
}

/**
 * 读取并更新GPS数据
 */
void updateGpsData() {
  while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
      if (gps.location.isValid()) {
        gpsLat = gps.location.lat();
        gpsLng = gps.location.lng();
      }
      
      if (gps.altitude.isValid()) {
        gpsAlt = gps.altitude.meters();
      }
      
      if (gps.satellites.isValid()) {
        gpsSats = gps.satellites.value();
      }
    }
  }
  
  // 检测GPS信号
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("未检测到GPS信号");
  }
}

/**
 * 更新辐射传感器数据(CPM)
 * @param currentMillis 当前时间
 */
void updateRadiationData(unsigned long currentMillis) {
  if (currentMillis - previousMillis > LOG_PERIOD) {
    previousMillis = currentMillis;
    cpm = counts * multiplier;
    Serial.print("辐射值(CPM): ");
    Serial.println(cpm);
    counts = 0;
  }
}

/**
 * 读取光照传感器数据
 */
void readLightData() {
  if (2 == BH1750_Read(BH1750_ADDR)) {
    if (buff[0] == 255 && buff[1] == 255) {
      val = 65535;
    } else {
      val = ((buff[0] << 8) | buff[1]) / 1.2; // 根据芯片手册计算
    }
    Serial.print("光照度: ");
    Serial.print(val);
    Serial.println(" lx");
  }
}

/**
 * 构建并发送传感器数据消息
 */
void buildAndSendMessage() {
  char buffer[128]; // 用于存储最终消息
  char tempStr[10];  // 温度字符串
  char luxStr[10];   // 光照字符串
  char gpsLatStr[15]; // GPS纬度字符串
  char gpsLngStr[15]; // GPS经度字符串
  char gpsAltStr[10]; // GPS高度字符串
  char gpsSatsStr[5]; // GPS卫星数字符串
  
  // 转换所有数据为字符串
  sprintf(buffer, "%d", cpm);
  sprintf(tempStr, "%d", (int)sensors.getTempCByIndex(0));
  sprintf(luxStr, "%d", val);
  dtostrf(gpsLat, 8, 6, gpsLatStr);
  dtostrf(gpsLng, 8, 6, gpsLngStr);
  dtostrf(gpsAlt, 4, 1, gpsAltStr);
  sprintf(gpsSatsStr, "%d", gpsSats);
  
  // 构建完整消息
  strcat(buffer, "cpm, ");
  strcat(buffer, tempStr);
  strcat(buffer, "℃, ");
  strcat(buffer, luxStr);
  strcat(buffer, "lx, lat:");
  strcat(buffer, gpsLatStr);
  strcat(buffer, ", lng:");
  strcat(buffer, gpsLngStr);
  strcat(buffer, ", alt:");
  strcat(buffer, gpsAltStr);
  strcat(buffer, "m, sats:");
  strcat(buffer, gpsSatsStr);
  
  // 发送消息
  publishMessage(buffer);
  Serial.print("发送数据: ");
  Serial.println(buffer);
}

/**
 * 主循环
 */
void loop() {
  unsigned long currentMillis = millis();
  
  // 1. MQTT客户端轮询
  mqttClient.poll();
  
  // 2. 读取和更新各传感器数据
  updateGpsData();
  updateRadiationData(currentMillis);
  sensors.requestTemperatures(); // 请求温度数据
  readLightData();
  
  // 3. 打包并发送所有传感器数据
  buildAndSendMessage();
  
  // 适当延时，减少CPU负载
  delay(500);
}

