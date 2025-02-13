#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#define LOG_PERIOD 15000  //Logging period in milliseconds, recommended value 15000-60000.
#define MAX_PERIOD 60000  //Maximum logging period without modifying this sketch
#include <OneWire.h>  // 调用OneWire库
#include <DallasTemperature.h>  // 调用DallasTemperature库
#define ONE_WIRE_BUS 4  // 定义18B20数据口连接arduino的 2 脚
#include <Wire.h>
OneWire oneWire(ONE_WIRE_BUS);  // 初始连接在单总线上的单总线设备
DallasTemperature sensors(&oneWire);
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
int BH1750address = 0x23;
byte buff[2];
uint16_t val = 0;
unsigned long counts;          //variable for GM Tube events
unsigned long cpm;             //variable for CPM
unsigned int multiplier;       //variable for calculation CPM in this sketch
unsigned long previousMillis;  //variable for time measurement 
//  设置WIFI
char ssid[] = "wifi_name";
char pass[] = "wifi_password";
//  设置MQTT(EMQX)
char mqtt_user[] = "user";
char mqtt_pass[] = "password";
const char broker[] = "mqtt_server_address";
int port = 1883;
//  指定发送主题（Arduino为信息发送方）
const char publish_topic[] = "unity/test";

void tube_impulse() {  //subprocedure for capturing events from Geiger Kit
  counts++;
}

void publishMessage(char* buffer) {
  // 创建一个 JSON 对象
  StaticJsonDocument<200> doc;
  doc["msg"] = buffer;
  // 将 JSON 对象序列化为字符串
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  // 向指定的主题发送 JSON 字符串
  mqttClient.beginMessage(publish_topic);
  mqttClient.print(jsonBuffer);
  mqttClient.endMessage();
  // 每隔一段时间发送一次
  delay(5000);
}

int BH1750_Read(int address) 
{
  int i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while (Wire.available())
  {
    buff[i] = Wire.read();  // receive one byte
    i++;
  }
  Wire.endTransmission();
  return i;
}
 
void BH1750_Init(int address)
{
  Wire.beginTransmission(address);
  Wire.write(0x10);
  Wire.endTransmission();
}


void setup() {
  counts = 0;
  cpm = 0;
  multiplier = MAX_PERIOD / LOG_PERIOD;  //calculating multiplier, depend on your log period
  Wire.begin();
  BH1750_Init(BH1750address);
  Serial.begin(9600);
  sensors.begin(); // 初始化总线
  pinMode(2,INPUT);
  attachInterrupt(digitalPinToInterrupt(2), tube_impulse, FALLING);  //define external interrupts
  while (!Serial) {
    ;
  }
  //  连接网络
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }
  Serial.println("You're connected to the network");
  Serial.println();
  //  连接MQTT
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);
  Serial.print("Attempting to connect to the MQTT broker: ");
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1)
      ;
  }
  Serial.println("You're connected to the MQTT broker!");
}
 
void loop() {
  unsigned long currentMillis = millis();
  char buffer1[24];
  char buffer2[10];
  char buffer3[10];
  char u1[]="cpm, ";
  char u2[]="℃, ";
  char u3[]="lx";
  unsigned long rd;
  if (currentMillis - previousMillis > LOG_PERIOD) {
    previousMillis = currentMillis;
    cpm = counts * multiplier;
    Serial.println(cpm);
    counts = 0;
  }
  sprintf(buffer1, "%d", cpm);
  mqttClient.poll();
  sensors.requestTemperatures(); 
  unsigned long temperature=sensors.getTempCByIndex(0);
  sprintf(buffer2,"%d",temperature);

  if (2 == BH1750_Read(BH1750address))
  {
    if (buff[0] == 255 && buff[1] == 255)
    {
      val = 65535;
    } else {
      val = ((buff[0] << 8) | buff[1]) / 1.2; //芯片手册中规定的数值计算方式
    }
    Serial.print(val, DEC);
    Serial.println("[lx]"); 
  }
  delay(500);sprintf(buffer3,"%d",val);
  strcat(buffer1,u1);
  strcat(buffer1,buffer2);
  strcat(buffer1,u2);
  strcat(buffer1,buffer3);
  strcat(buffer1,u3);
  publishMessage(buffer1);
  Serial.println(buffer1);
}

