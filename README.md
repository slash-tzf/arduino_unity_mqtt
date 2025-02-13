# Arduino Unity MQTT Project

This project collects data from sensors (Geiger Counter, BH1750 light sensor, and DS18B20 temperature sensor) and sends the data to an MQTT broker. The WiFi and MQTT connection settings are configured below.

## Setup

### WiFi Setup
Modify the following variables with your WiFi credentials:
```cpp
char ssid[] = "wifi_name";        // Your WiFi network name (SSID)
char pass[] = "wifi_password";    // Your WiFi password
```
### MQTT Setup
```cpp
char mqtt_user[] = "user";        //Mqtt Server user_name
char mqtt_pass[] = "password";    //Mqtt Server password
const char broker[] = "mqtt_server_address";  //Mqtt Server address
```
