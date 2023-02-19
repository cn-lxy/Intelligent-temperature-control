#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>

#include "PubSubClient.h"
#include "WiFi.h"
#include <aliyun_mqtt.h>
#include <ArduinoJson.h>

#include "Ticker.h"

#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <OneWire.h>
#include <DallasTemperature.h>

/*------------------------------------------ GPIO --------------------------------------------------*/
// GPIO DS18B20
#define DS18B20_PIN 32

// GPIO OLED IIC引脚
static const uint8_t ESP32_SCL = 22;
static const uint8_t ESP32_SDA = 21;

// GPIO 风扇引脚
#define INA_PIN 13
#define INB_PIN 12

// GPIO 制冷和制热
#define HOT_PIN 14
#define COLD_PIN 27

// TAG 温度加减按钮
#define BTN_ADD 26
#define BTN_SUB 25

// Board LED
#define LED 2

/*------------------------------------------ WiFi & MQTT --------------------------------------------*/
#define WIFI_SSID "DESKTOP-LXY" // wifi名
#define WIFI_PASSWD "12345678"  // wifi密码

#define PRODUCT_KEY "a1nKID4oRsp"                        // 产品ID
#define DEVICE_NAME "ESP32"                              // 设备名
#define DEVICE_SECRET "a687a3d6e96dc9272e90450841d8d564" // 设备key
#define REGION_ID "cn-shanghai"

// 服务端消息订阅Topic
#define ALINK_TOPIC_PROP_SET "/a1nKID4oRsp/" DEVICE_NAME "/user/get"
// 属性上报Topic
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
// 设备post上传数据要用到一个json字符串, 这个是拼接postJson用到的一个字符串
#define ALINK_METHOD_PROP_POST "thing.event.property.post"
// 这是post上传数据使用的模板
#define ALINK_BODY_FORMAT "{\"id\":\"%u\",\"version\":\"1.0.0\",\"method\":\"%s\",\"params\":%s}"

/*----------------------------------------- 类型定义 -------------------------------------------------*/
typedef struct
{
    volatile int envTemperature = 0; // 环境温度
} InnerData;                         // 内部数据结构体

typedef struct
{
    volatile int setTemperature = 0; // 设置温度
    volatile int mode = 0;           // 模式 => [0:制冷, 1:加热]
    volatile int windSpeed = 0;      // 风速等级 0: 关闭; 1: 低速; 2: 中速; 3: 高速;
} AliData;

/*----------------------------------------- 全局变量 -------------------------------------------------*/
InnerData innerData;       // 内部数据
AliData aliData;           // 接收云端数据
int postMsgId = 0;         // 记录已经post了多少条
TickType_t timeout = 1000; // 获取锁超时时间

/*------------------------------------------- 全局对象 -----------------------------------------------*/
WiFiClient espClient;               // 创建网络连接客户端
PubSubClient mqttClient(espClient); // 通过网络客户端连接创建mqtt连接客户端

U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*SCL*/ ESP32_SCL, /*SDA*/ ESP32_SDA, /*reset*/ U8X8_PIN_NONE); // 构造

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// 两个数据结构体的互斥锁
SemaphoreHandle_t xMutexInner = NULL; // 内部数据锁
SemaphoreHandle_t xMutexOuter = NULL; // 外部数据锁

/*----------------------------------------- 函数定义 -------------------------------------------------*/
void setup(void);                                               // [sys] 初始化函数
void loop(void);                                                // [sys] 循环函数
void modeChange(void);                                          // [driver] 模式切换
void keysHandler(void);                                         // [driver] 按键处理
void fanControl(void);                                          // [driver] 风扇控制
void oledLog(String s);                                         // [log] 屏幕打印日志
void setupWifi(void);                                           // [WIFI] 连接WIFI相关函数
void clientReconnect();                                         // [WIFI] 重连函数, 如果客户端断线,可以通过此函数重连
void callback(char *topic, byte *payload, unsigned int length); // [data]收到消息回调
void mqttCheck(void);                                           // [WIFI] 检测有没有断线

/*----------------------------------------- 任务定义 -------------------------------------------------*/
void sensorGetTask(void *ptParams); // [task] 获取传感器数据
void displayTask(void *ptParams);   // [task] 屏幕显示
void netCheckTask(void *ptParams);  // [task] 网络检查
void driverTask(void *ptParams);    // [task] 外设控制
void uploadData(void *ptParams);    // [task] 数据上传
void logTask(void *ptParams);       // [task] 日志打印
#endif