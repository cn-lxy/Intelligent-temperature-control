#include <Arduino.h>
#include <WiFi.h>

static WiFiClient espClient;
#include <ArduinoJson.h>

#include <AliyunIoTSDK.h>
AliyunIoTSDK iot;

//设备三元组外加一个区域ID
#define PRODUCT_KEY "a1nKID4oRsp"
#define DEVICE_NAME "ESP32"
#define DEVICE_SECRET "a687a3d6e96dc9272e90450841d8d564"
#define REGION_ID "cn-shanghai"

//无线网络名称及密码
#define WIFI_SSID "HUAWEI nova4"
#define WIFI_PASSWD "12345678"

void wifiInit(const char *ssid, const char *passphrase)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    WiFi.setAutoConnect (true);
    WiFi.setAutoReconnect (true);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }
    Serial.println("Connected to AP");
}

void powerCallback(JsonVariant p)
{
    int PowerSwitch = p["PowerSwitch"];
    Serial.println("PowerSwitch:" + PowerSwitch);
}

void temperatureCallback(JsonVariant p)
{
    int temperature = p["temperature"];
    Serial.println("temperature:" + temperature);
}


void setup() {
	Serial.begin(115200);

	wifiInit(WIFI_SSID, WIFI_PASSWD);

	AliyunIoTSDK::begin(espClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, REGION_ID);

	// 绑定属性回调
	AliyunIoTSDK::bindData("PowerSwitch", powerCallback);
    AliyunIoTSDK::bindData("temperature", temperatureCallback);
}

unsigned long lastMsMain = 0;
void loop() {
	AliyunIoTSDK::loop();
	if (millis() - lastMsMain >= 5000)
	{
		lastMsMain = millis();
		// 发送事件到阿里云平台
		// AliyunIoTSDK::sendEvent("xxx"); 
		// 发送模型属性到阿里云平台
		AliyunIoTSDK::send("temperature", 37.0);
	}
}

