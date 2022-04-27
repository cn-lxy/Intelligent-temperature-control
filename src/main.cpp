#include <Arduino.h>
#include <ArduinoJson.h>
#include <aliyun_mqtt.h>
#include "PubSubClient.h"
#include "WiFi.h"
#include "Ticker.h"

#define WIFI_SSID "HUAWEI nova4"       //wifi名
#define WIFI_PASSWD "12345678" //wifi密码

#define PRODUCT_KEY "a1nKID4oRsp"                        //产品ID
#define DEVICE_NAME "ESP32"                     //设备名
#define DEVICE_SECRET "a687a3d6e96dc9272e90450841d8d564" //设备key
#define REGION_ID "cn-shanghai"

// 服务端消息订阅Topic
#define ALINK_TOPIC_PROP_SET "/a1nKID4oRsp/" DEVICE_NAME "/user/get"
// 属性上报Topic
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
// 设备post上传数据要用到一个json字符串, 这个是拼接postJson用到的一个字符串
#define ALINK_METHOD_PROP_POST "thing.event.property.post"
// 这是post上传数据使用的模板
#define ALINK_BODY_FORMAT "{\"id\":\"%u\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s}"

#define LED_B 2 // 定义LED灯的引脚

int postMsgId = 0; //记录已经post了多少条
Ticker tim1;       //这个定时器是为了每5秒上传一次数据

/*------------------------------------------------------------------------------------------*/

WiFiClient espClient;               //创建网络连接客户端
PubSubClient mqttClient(espClient); //通过网络客户端连接创建mqtt连接客户端

//连接WIFI相关函数
void setupWifi()
{
  delay(10);
  Serial.println("连接WIFI");
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (!WiFi.isConnected())
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("OK");
  Serial.println("Wifi连接成功");
}

//重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect()
{
  while (!mqttClient.connected()) //再重连客户端
  {
    Serial.println("reconnect MQTT...");
    if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.println("failed");
      Serial.println(mqttClient.state());
      Serial.println("try again in 5 sec");
      delay(5000);
    }
  }
}
//mqtt发布post消息(上传数据)
void mqttPublish()
{
  if (mqttClient.connected())
  {
    //先拼接出json字符串
    char param[32];
    char jsonBuf[128];
    // sprintf(param, "{\"LightSwitch\":%d}", digitalRead(LED_B)); //我们把要上传的数据写在param里
    sprintf(param, "{\"temperature\":%.1f}", 36.6); //我们把要上传的数据写在param里
    postMsgId += 1;
    sprintf(jsonBuf, ALINK_BODY_FORMAT, postMsgId, ALINK_METHOD_PROP_POST, param);
    //再从mqtt客户端中发布post消息
    if (mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf))
    {
      Serial.print("Post message to cloud: ");
      Serial.println(jsonBuf);
    }
    else
    {
      Serial.println("Publish message to cloud failed!");
    }
  }
}
// 收到消息回调
void callback(char *topic, byte *payload, unsigned int length)
{
//   if (strstr(topic, ALINK_TOPIC_PROP_SET))
//   {
    Serial.println("收到下发的命令主题:");
    Serial.println(topic);
    Serial.println("下发的内容是:");
    payload[length] = '\0'; //为payload添加一个结束附,防止Serial.println()读过了
    Serial.println((char *)payload);

    // 接下来是收到的json字符串的解析
    DynamicJsonDocument doc(100);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.println("parse json failed");
      return;
    }
    JsonObject setAlinkMsgObj = doc.as<JsonObject>();
    serializeJsonPretty(setAlinkMsgObj, Serial);
    Serial.println();

    // 这里是一个点灯小逻辑
    int lightSwitch = setAlinkMsgObj["params"]["LightSwitch"];
    digitalWrite(LED_B, lightSwitch);
    mqttPublish(); //由于将来做应用可能要获取灯的状态,所以在这里发布一下
//   }
}

void setup()
{
  pinMode(LED_B, OUTPUT);
  Serial.begin(115200);
  setupWifi();
  if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
  {
    Serial.println("MQTT服务器连接成功!");
  };
  //! 订阅Topic !!这是关键!!
  mqttClient.subscribe(ALINK_TOPIC_PROP_SET);
  mqttClient.setCallback(callback); //绑定收到set主题时的回调(命令下发回调)
  tim1.attach(10, mqttPublish);     //启动每5秒发布一次消息
}

void loop()
{
  //检测有没有断线
  if (!WiFi.isConnected()) //先看WIFI是否还在连接
  {
    setupWifi();
  }
  else //如果WIFI连接了,
  {
    if (!mqttClient.connected()) //再看mqtt连接了没
    {
      Serial.println("mqtt disconnected!Try reconnect now...");
      Serial.println(mqttClient.state());
      clientReconnect();
    }
  }

  //mqtt客户端监听
  mqttClient.loop();
}