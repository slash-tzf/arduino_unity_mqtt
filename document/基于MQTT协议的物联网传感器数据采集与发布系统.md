# 基于MQTT协议的物联网传感器数据采集与发布系统



随着物联网（IoT）技术的迅猛发展，智能设备之间的互联互通已成为现代技术应用的一个重要方向。在物联网系统中，传感器起着至关重要的作用，能够实时收集环境数据，提供智能决策的基础。本报告基于一段Arduino代码，结合MQTT协议和多个传感器的应用，分析其原理和实现方式。报告将详细介绍MQTT协议的工作原理，涉及的传感器类型及其应用，代码的详细分析，以及如何实现物联网设备的数据采集和远程发布。

## 一、MQTT协议原理

### 1.1 什么是MQTT？

MQTT（Message Queuing Telemetry Transport）是一种轻量级的消息传输协议，广泛应用于物联网（IoT）领域。MQTT最初由IBM设计，旨在实现低带宽、低功耗、可靠的消息传输。其核心特点是基于发布/订阅（Publish/Subscribe）模型，这使得它特别适合于设备之间的通信，尤其是对于那些需要频繁交换小量数据的设备。

### 1.2 MQTT协议工作原理

MQTT协议采用客户端-服务器模式，其中服务器通常被称为“MQTT代理”（Broker）。客户端通过连接到MQTT代理来发布（publish）消息或订阅（subscribe）消息。

1. **连接（Connect）**：客户端首先需要向MQTT代理发送连接请求。请求中包含客户端的身份信息（如用户名和密码），以及连接的相关参数（如最大消息大小、保持连接的时间等）。
2. **发布（Publish）**：客户端可以将消息发布到一个主题（Topic），这个主题可以是任何字符串，如“sensors/temperature”。
3. **订阅（Subscribe）**：客户端可以订阅特定的主题，接收来自代理的消息。
4. **消息传递（Message Delivery）**：当有消息发布到某个主题时，所有订阅该主题的客户端都会接收到该消息。消息的传递通过QoS（服务质量）级别进行控制，确保消息传输的可靠性。
5. **断开（Disconnect）**：当客户端不再需要通信时，可以断开与MQTT代理的连接。

![image-20250224191520695](https://raw.githubusercontent.com/slash-tzf/image/main/20250224191553360.png)

### 1.3 MQTT的优势

- **轻量级**：MQTT协议设计简单，传输头部小，非常适合资源有限的嵌入式设备。
- **低带宽消耗**：适合低带宽、高延迟的网络环境，节省了传输数据的带宽。
- **高可靠性**：MQTT支持消息的确认机制，可以确保消息传输的可靠性，支持三种不同的服务质量（QoS）级别。
- **实时性**：通过持久会话和订阅机制，能够保证数据的即时传递。
### 1.4 如何在服务器上部署 MQTT Broker

  部署 MQTT Broker 是使用 MQTT 协议进行通信的关键步骤。下面将介绍如何在服务器上部署一个常用的 MQTT Broker——EMQX（Erlang/Enterprise MQTT Broker）。

  #### 1.4.1 安装 EMQX

  EMQX 是一个开源的 MQTT Broker，支持高并发和高吞吐量，非常适合大规模物联网应用。以下是安装步骤：

  ##### 1.4.1.1 使用二进制文件安装（适用于 Linux、macOS）

  1. **下载 EMQX** 访问 [EMQX 官方网站](https://www.emqx.io/downloads)下载适合你操作系统的二进制文件。

     ```bash
     wget https://www.emqx.io/downloads/emqx-4.4.0/emqx-4.4.0-ubuntu20.04_amd64.deb
     ```

  2. **安装 EMQX** 对于 Ubuntu 系统，可以使用 `dpkg` 命令进行安装：

     ```bash
     sudo dpkg -i emqx-4.4.0-ubuntu20.04_amd64.deb
     ```

     对于其他 Linux 发行版或 macOS，可以根据平台选择合适的安装包（tar.gz 或其他）并解压。

  ##### 1.4.1.2 使用 Docker 安装

  如果你更喜欢使用 Docker，可以通过以下命令快速部署 EMQX：

  1. 拉取 EMQX Docker 镜像：

     ```bash
     docker pull emqx/emqx
     ```

  2. 运行 EMQX 容器：

     ```bash
     docker run -d --name emqx -p 1883:1883 -p 8083:8083 emqx/emqx
     ```

     这条命令将启动一个 EMQX 容器，并将 MQTT 默认端口 `1883` 和 Web 管理端口 `8083` 映射到主机上。

  #### 1.4.2 配置 MQTT Broker

  安装完成后，EMQX 的默认配置已经可以满足大部分需求，但你可能需要根据自己的应用场景进行一些调整。

  ##### 1.4.2.1 配置文件路径

  在 Linux 系统中，EMQX 的配置文件通常位于 `/etc/emqx/` 或 `/opt/emqx/etc/` 目录下，主要配置文件是 `emqx.conf` 和 `vm.args`。

  - **emqx.conf**：主要配置 MQTT Broker 的参数，例如监听端口、认证方式、消息存储、日志设置等。
  - **vm.args**：配置 Erlang 虚拟机（VM）的启动参数，包含内存限制、进程数等。

  ##### 1.4.2.2 配置示例

  以下是修改 `emqx.conf` 文件的一些示例：

  - **设置 MQTT 默认端口**：

    ```ini
    listener.tcp.default = 1883
    ```

  - **启用 Web 管理控制台**：

    ```ini
    management.listener = 8083
    ```

  - **修改最大连接数和消息缓存**：

    ```ini
    mqtt.max_client_id_length = 128
    mqtt.max_inflight_messages = 100
    ```

  ##### 1.4.2.3 配置认证与安全

  EMQX 提供了多种认证方式（如用户名/密码认证、客户端证书认证等），可以在配置文件中启用这些选项。

  - **启用用户名/密码认证**：

    修改 `emqx.conf` 中的认证配置：

    ```ini
    auth.username = admin
    auth.password = public
    ```

  - **启用 TLS/SSL 加密**：

    修改监听端口配置，启用 TLS：

    ```ini
    listener.tcp.ssl = 8883
    listener.tcp.ssl.keyfile = /etc/emqx/ssl/key.pem
    listener.tcp.ssl.certfile = /etc/emqx/ssl/cert.pem
    ```

  #### 1.4.3 启动 MQTT Broker

  EMQX 安装完成后，使用以下命令启动服务：

  ```bash
  sudo systemctl start emqx
  ```

  或者，如果你使用的是 Docker 部署，可以直接运行：

  ```bash
  docker start emqx
  ```

  你可以使用以下命令查看 EMQX 服务的状态：

  ```bash
  sudo systemctl status emqx
  ```

  #### 1.4.4 管理和监控 MQTT Broker

  EMQX 提供了 Web 管理界面，可以通过访问 `http://<server-ip>:8083` 来登录控制台。默认的管理员用户名和密码为：

  - 用户名：`admin`
  - 密码：`public`

  此外，EMQX 还支持通过 REST API 进行远程管理，你可以在官方文档中找到详细的 API 说明。

  #### 1.4.5 连接 MQTT Broker

  配置完成后，可以使用 MQTT 客户端（如 **MQTT.fx**、**mosquitto_pub/sub** 等）来连接你部署的 Broker，开始发布和订阅消息。

  例如，使用 **mosquitto_pub** 命令发布消息：

  ```bash
  mosquitto_pub -h <server-ip> -t "sensors/temperature" -m "22.5"
  ```

  使用 **mosquitto_sub** 命令订阅消息：

  ```bash
  mosquitto_sub -h <server-ip> -t "sensors/temperature"
  ```

  #### 1.4.6 配置扩展

  根据应用需求，你可以进一步配置高可用性、集群模式、负载均衡等功能。EMQX 支持多种扩展，包括分布式部署，集群功能非常适合大规模应用场景。

  通过上述步骤，就可以成功地在服务器上部署并配置 MQTT Broker，从而开始使用 MQTT 协议进行设备间的通信。

进入18083端口即可看到控制台

![image-20250224203740204](https://raw.githubusercontent.com/slash-tzf/image/main/20250224213548630.png)

## 二、相关传感器介绍

本系统使用了多个传感器，包括温度传感器、光照传感器和辐射传感器。下面将介绍这些传感器的基本工作原理及其应用。

### 2.1 温度传感器：DS18B20

DS18B20是一款由Dallas Semiconductor（现为Maxim Integrated）公司生产的数字温度传感器。该传感器使用1-Wire协议进行通信，支持精度为0.5°C的温度测量，测量范围为-55°C到+125°C。DS18B20具有较低的功耗，并且支持多点传感器网络连接。

- **工作原理**：DS18B20通过1-Wire总线与主控设备（如Arduino）进行通信，发送和接收数据。传感器内部包含一个高精度的温度传感器芯片，该芯片通过数字化信号输出温度值。
- **应用**：DS18B20广泛应用于环境监测、智能家居、气象站等领域，用于监测温度变化。

### 2.2 光照传感器：BH1750

BH1750是一款数字化光照传感器，能够测量环境光的强度。它使用I2C接口与主控设备进行通信，能够在高达16位分辨率的范围内提供准确的光强测量。

- **工作原理**：BH1750内部集成了一个光电二极管，通过检测周围环境的光线强度，将其转换为电信号，进而通过I2C接口传输给主控设备。
- **应用**：该传感器主要应用于智能照明、自动窗帘、环境光检测等领域。

### 2.3 辐射传感器

辐射传感器主要用于检测放射性物质的辐射强度，常用于核辐射监测、环境污染监测等领域。该系统使用了Geiger-Müller（GM）管来检测辐射事件。

- **工作原理**：GM管是一种能够检测粒子辐射（如α、β、γ射线）的设备。当辐射粒子进入GM管并引发电离事件时，管内的气体会发生电离，产生脉冲信号。这个脉冲信号可以被计数器统计，从而确定辐射的强度。
- **应用**：用于核辐射监测、环境污染检测等场所。

## 三、代码分析

### 3.0 开发环境

采用VScode + platformIO环境开发，方便依赖包管理

从 [vscode官网](https://code.visualstudio.com/download)下载符合操作系统的版本后下载插件C/C++、platform插件

![image-20250224192138261](https://raw.githubusercontent.com/slash-tzf/image/main/20250224193350882.png)

选择导入工程即可阅读源码，以及下载代码到开发板

![image-20250224193039758](https://raw.githubusercontent.com/slash-tzf/image/main/20250224193350883.png)





### 3.1完整代码

```cpp
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


```

### 3.2 代码概述

本代码的目标是从多个传感器（温度、光照、辐射）采集数据，并通过MQTT协议将这些数据发布到服务器。代码使用Arduino平台进行开发，利用WiFi模块实现与网络的连接，并通过MQTT协议与远程服务器进行数据交换。

### 3.3 传感器初始化与数据采集

1. **温度传感器（DS18B20）**：在`setup`函数中通过`sensors.begin()`初始化DS18B20传感器，然后通过`sensors.requestTemperatures()`来请求温度数据。在`loop`函数中，调用`sensors.getTempCByIndex(0)`获取当前温度，并将其存储到`buffer2`数组中。
2. **光照传感器（BH1750）**：在`setup`函数中初始化光照传感器，并通过`BH1750_Read()`函数读取传感器数据。该函数通过I2C接口向传感器发送命令并获取数据，将光照强度存储在`val`变量中。
3. **辐射传感器（GM管）**：通过外部中断（`attachInterrupt`）捕捉辐射事件，每当GM管检测到辐射事件时，`tube_impulse()`函数会被调用，`counts`变量会增加。通过计算事件发生的次数，得出辐射强度。

### 3.4 数据发布

通过MQTT协议，将采集到的传感器数据发布到指定的主题`unity/test`。数据通过`publishMessage()`函数发送，该函数将数据封装为JSON格式，并通过MQTT客户端发送出去。JSON格式的数据结构如下：

```json
{
  "msg": "cpm, 100, ℃, 22, lx"
}
```

其中，`cpm`表示辐射计数，`℃`表示温度值，`lx`表示光照强度。每5秒钟通过`publishMessage()`函数向MQTT服务器发送一次数据。

### 3.5 网络与MQTT连接

在`setup`函数中，首先通过WiFi连接到指定的网络，连接成功后通过MQTT客户端连接到MQTT服务器。如果连接失败，程序会持续尝试重新连接。

需要注意，该开发板搭载的芯片只支持2.4G频段的WiFi,不支持5G频段WiFi。目前大部分市面上的路由器都支持双频合一，会自动切换频段，如遇到无法连接，请先检查WiFi频段是否支持；若确定支持后仍然无法连接，请在路由器中打开最大兼容模式。


### 3.6 循环逻辑

在`loop`函数中，首先通过`millis()`获取当前时间，并根据设定的日志周期`LOG_PERIOD`计算辐射计数。每隔`LOG_PERIOD`毫秒，计算一次辐射强度（`cpm`）。然后获取传感器数据，并将数据发送到MQTT服务器。

[Github源码链接](https://github.com/slash-tzf/arduino_unity_mqtt)


## 4.基于Unity和VR的物联网数据可视化系统

### 4.1 Unity Oculus VR配置

#### 4.1.1 Unity下载以及安装

Unity 是一个跨平台的游戏引擎，广泛用于开发 2D 和 3D 游戏、虚拟现实（VR）、增强现实（AR）应用以及其他实时交互式内容。它提供了一个集成的开发环境（IDE），支持多种编程语言，最常用的是 C#。Unity 引擎的一个显著特点是其强大的跨平台能力，可以将游戏或应用发布到多个平台，包括 PC、移动设备、主机、Web 和 VR/AR 设备等。

[Unity Hub](https://unity.com/cn/download) 是使用Unity必须安装的软件，这个软件可以帮助我们管理Unity软件的版本和Unity项目。 进入下载页面。点击“下载”即可完成对unity hub的下载。

![image-20250224200907382](https://raw.githubusercontent.com/slash-tzf/image/main/20250224213548631.png)

在完成下载Unity Hub后，注册相关账号后请下载[Unity 2020.3.30f1](https://unity.cn/release-notes/full/2020/2020.3.30f1)版本。

> [!WARNING]
>
> **！！！请勿下载其他版本，本文档提供的代码均基于此版本构建，包括 quest SDK！！！**



#### 4.1.2 Oculus SDK下载以及安装

关于Oculus SDK下载以及基础操作部分请参考[硬核的XR王师傅的quest教程](https://www.bilibili.com/video/BV1Xr4y1p7Ah/)

> [!WARNING]
>
> **！！！由于quest SDK的版本迭代速度非常快，且内容差异巨大，请不要下载最新的版本，请下载教程中的版本[Oculus SDK 38.0](https://developers.meta.com/horizon/downloads/package/oculus-platform-sdk/38.0),否则无法运行代码！！！**



### 4.2 unity MQTT客户端

该部分代码主要由mqtt_manager.cs文件实现。

这段代码主要实现了一个基于 MQTT 协议的 Unity 脚本，它从 MQTT 代理接收消息，并根据接收到的 JSON 数据在 Unity 场景中生成一个动态的“数据球”和相应的标签。代码中使用了 TextMeshPro 用于显示标签，XR Interaction Toolkit 用于处理与数据球的交互。接下来，我们将分块详细解释代码的功能和实现。

#### 4.2.1. 引用的命名空间

```csharp
using System;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Concurrent;
using UnityEngine;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using Newtonsoft.Json.Linq;
using UnityEngine.XR.Interaction.Toolkit;
using TMPro; // 添加 TextMeshPro 命名空间
```

- `System` 和 `System.Text`：用于字符串处理和编码。
- `System.Threading.Tasks`：用于异步任务处理。
- `System.Collections.Concurrent`：用于线程安全的队列（`ConcurrentQueue`），它用于处理接收到的 MQTT 消息。
- `UnityEngine`：Unity 引擎的核心命名空间，用于处理 Unity 相关功能。
- `MQTTnet`：用于实现 MQTT 客户端和通信功能。
- `Newtonsoft.Json.Linq`：用于解析 JSON 数据。
- `UnityEngine.XR.Interaction.Toolkit`：Unity 的 XR (扩展现实) 工具包，用于实现交互功能。
- `TMPro`：TextMeshPro 用于显示文本标签。

#### 4.2.2. `mqtt_manager` 类定义

```csharp
public class mqtt_manager : MonoBehaviour
{
    private IMqttClient mqttClient;
    private ConcurrentQueue<string> messageQueue = new ConcurrentQueue<string>();

    // MQTT 配置
    private string brokerAddress = "you address";
    private int brokerPort = 1883;
    private string topic = "unity/test";

    // 地形引用
    public Terrain terrain;

    // TextMeshPro 预制件
    public TextMeshPro textMeshProPrefab;

    // 用于管理唯一的数据球和标签
    private GameObject dataPoint;
    private DataPointHandler dataPointHandler;
```

#### 变量解释：

- `mqttClient`：用于表示 MQTT 客户端的接口。
- `messageQueue`：一个线程安全的队列，用来存储从 MQTT 代理接收到的消息。
- `brokerAddress`、`brokerPort` 和 `topic`：用于配置连接的 MQTT 代理的地址、端口和订阅的主题。
- `terrain`：Unity 地形对象，用于获取地形的高度数据。
- `textMeshProPrefab`：TextMeshPro 文本预制件，用于显示消息。
- `dataPoint` 和 `dataPointHandler`：用来管理和操作数据球和标签的对象。

#### 4.2.3. `Start` 方法

```csharp
async void Start()
{
    if (terrain == null)
    {
        Debug.LogError("请在 Inspector 中设置地形对象！");
        return;
    }

    if (textMeshProPrefab == null)
    {
        Debug.LogError("请在 Inspector 中设置 TextMeshPro 预制件！");
        return;
    }

    // 创建 MQTT 客户端
    var factory = new MqttFactory();
    mqttClient = factory.CreateMqttClient();

    var options = new MqttClientOptionsBuilder()
        .WithTcpServer(brokerAddress, brokerPort)
        .WithClientId(Guid.NewGuid().ToString())
        .WithCleanSession()
        .Build();

    // 设置事件处理
    mqttClient.UseConnectedHandler(async e =>
    {
        Debug.Log("已连接到 MQTT 代理");
        await mqttClient.SubscribeAsync(new MqttTopicFilterBuilder().WithTopic(topic).Build());
        Debug.Log($"已订阅主题: {topic}");
    });

    mqttClient.UseDisconnectedHandler(e =>
    {
        Debug.LogWarning("与 MQTT 代理断开连接");
    });

    mqttClient.UseApplicationMessageReceivedHandler(e =>
    {
        string payload = Encoding.UTF8.GetString(e.ApplicationMessage.Payload);
        Debug.Log($"收到消息: {payload}");
        messageQueue.Enqueue(payload); // 将消息加入队列
    });

    try
    {
        await mqttClient.ConnectAsync(options);
    }
    catch (Exception ex)
    {
        Debug.LogError($"连接到 MQTT 代理失败: {ex.Message}");
    }
}
```

#### 解释：

- `Start` 方法是 Unity 的生命周期方法之一，在场景开始时调用。
- 检查 `terrain` 和 `textMeshProPrefab` 是否设置，如果未设置则打印错误信息。
- 创建 MQTT 客户端，并配置连接到指定的 MQTT 代理（`brokerAddress` 和 `brokerPort`）。
- 配置并处理 MQTT 连接、断开连接和接收到消息的事件：
  - 连接成功后，订阅指定的 `topic`。
  - 断开连接时输出警告信息。
  - 接收到消息时，将其解码并放入 `messageQueue`。

#### 4.2.4. `Update` 方法

```csharp
void Update()
{
    // 从队列中取出所有消息并处理
    while (messageQueue.TryDequeue(out string jsonString))
    {
        HandleReceivedJsonOnMainThread(jsonString);
    }
}
```

#### 解释：

- `Update` 是 Unity 的生命周期方法之一，帧频执行。
- 它从 `messageQueue` 中取出所有待处理的 MQTT 消息，并调用 `HandleReceivedJsonOnMainThread` 方法处理每一条消息。

#### 4.2.5. `HandleReceivedJsonOnMainThread` 方法

```csharp
void HandleReceivedJsonOnMainThread(string jsonString)
{
    try
    {
        // 解析 JSON 数据
        JObject json = JObject.Parse(jsonString);

        // 获取 "msg" 字段的内容
        string msg = json["msg"]?.ToString();
        float value = json["value"]?.Value<float>() ?? 0;

        // 动态计算基于地形的默认坐标
        Vector3 defaultPosition = GetDefaultTerrainPosition();

        // 如果 JSON 中有指定的坐标字段，则优先使用
        Vector3 position = defaultPosition;
        JObject positionJson = json["position"] as JObject;
        if (positionJson != null)
        {
            float x = positionJson["x"]?.Value<float>() ?? defaultPosition.x;
            float y = positionJson["y"]?.Value<float>() ?? defaultPosition.y;
            float z = positionJson["z"]?.Value<float>() ?? defaultPosition.z;
            position = new Vector3(x, y, z);
        }

        // 创建或更新数据展示对象
        if (dataPoint == null)
        {
            CreateDataPoint(position, msg, value);
        }
        else
        {
            UpdateDataPoint(msg, value);
        }
    }
    catch (Exception ex)
    {
        Debug.LogError($"处理 JSON 时出错: {ex.Message}");
    }
}
```

#### 解释：

- 这个方法会解析从 MQTT 消息中收到的 JSON 数据。
- 获取 `msg` 和 `value` 字段的信息，并根据这些数据生成或更新数据展示对象（数据球和标签）。
- 如果 `position` 字段存在，则使用该位置；如果不存在，则使用计算出来的默认位置。

#### 4.2.6. `GetDefaultTerrainPosition` 方法

```csharp
Vector3 GetDefaultTerrainPosition()
{
    // 获取地形的基础坐标
    Vector3 terrainPosition = terrain.transform.position;

    // 获取地形的中心位置的 Y 高度
    float terrainHeight = terrain.SampleHeight(new Vector3(terrainPosition.x, 0, terrainPosition.z));

    // 返回地形基础坐标并将 Y 轴加 2
    return new Vector3(terrainPosition.x +500.6f, terrainHeight + 20f, terrainPosition.z+510.5f);
}
```

#### 解释：

- 计算地形上默认的位置：从地形的坐标位置中获取基础坐标，并通过 `SampleHeight` 获取 Y 轴的高度，最终返回一个新的 `Vector3` 作为默认位置。

#### 4.2.7. `CreateDataPoint` 方法

```csharp
void CreateDataPoint(Vector3 position, string msg, float value)
{
    // 创建数据球
    dataPoint = GameObject.CreatePrimitive(PrimitiveType.Sphere);
    dataPoint.transform.position = position;
    dataPoint.transform.localScale = Vector3.one * 0.5f;

    // 设置颜色
    Renderer renderer = dataPoint.GetComponent<Renderer>();
    renderer.material.color = Color.green;

    // 添加显示控制组件
    dataPointHandler = dataPoint.AddComponent<DataPointHandler>();
    dataPointHandler.Initialize(msg, value, textMeshProPrefab);

    // 添加 XR Simple Interactable 组件
    XRSimpleInteractable interactable = dataPoint.AddComponent<XRSimpleInteractable>();
    interactable.hoverEntered.AddListener(dataPointHandler.OnHoverEnter); // 悬停显示
    interactable.hoverExited.AddListener(dataPointHandler.OnHoverExit);   // 悬停隐藏
    interactable.selectEntered.AddListener(dataPointHandler.OnSelectToggle); // 按抓取键切换显示状态
}
```

#### 解释：

- 创建一个数据球（`PrimitiveType.Sphere`），设置它的大小和位置。
- 设置数据球的颜色为绿色。
- 添加一个 `DataPointHandler` 组件，用于处理显示控制逻辑，包括初始化文本标签。
- 添加 `XRSimpleInteractable` 组件，支持 XR 交互（如悬停和点击）。

#### 4.2.8. `UpdateDataPoint` 方法

```csharp
void UpdateDataPoint(string msg, float value)
{
    // 更新数据球和标签内容
    dataPointHandler.UpdateLabel(msg, value);
}
```

#### 解释：

- 如果数据球已创建，它将根据新的消息更新标签内容（`msg` 和 `value`）。

#### 4.2.9. `OnApplicationQuit` 方法

```csharp
async void OnApplicationQuit()
{
    if (mqttClient != null && mqttClient.IsConnected)
    {
        await mqttClient.DisconnectAsync();
        Debug.Log("已断开与 MQTT 代理的连接");
    }
}
```

#### 解释：

- 在应用退出时，如果 MQTT 客户端连接仍然存在，则断开连接。

这就是整个代码的详细解释，涵盖了如何使用 MQTT 协议将数据接入 Unity，并通过创建动态数据球和标签在场景中显示数据。

### 4.3场景创建

该部分比较简单，若没有相关基础请参考[【Unity教程】零基础带你从小白到超神](https://www.bilibili.com/video/BV1gQ4y1e7SS),场景比较简单，素材已经在工程内。