// #include "aliyunServer.h"
// #include <Arduino.h>
// #include "PubSubClient.h"
// #include "WiFi.h"
// #include <aliyun_mqtt.h>
// #include <ArduinoJson.h>


// WiFiClient espClient;               //创建网络连接客户端
// PubSubClient mqttClient(espClient); //通过网络客户端连接创建mqtt连接客户端

// unsigned int postMsgId = 0; //记录已经post了多少条

// void setupWiFi(void) {
//     delay(10);
//     Serial.println("连接WIFI");
//     WiFi.begin(WIFI_SSID, WIFI_PASSWD);
//     while (!WiFi.isConnected())
//     {
//         Serial.print(".");
//         delay(500);
//     }
//     Serial.println("OK");
//     Serial.println("Wifi连接成功");
// }

// void clientReconnect(void) {
//     while (!mqttClient.connected()) { //再重连客户端
//         Serial.println("reconnect MQTT...");
//         if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
//             Serial.println("connected");
//         } else {
//             Serial.println("failed");
//             Serial.println(mqttClient.state());
//             Serial.println("try again in 5 sec");
//             delay(5000);
//         }
//     }
// }

// // tag [发送消息] mqtt发布post消息
// void mqttPublish(char *param) {
//     if (mqttClient.connected()) {
//         //先拼接出json字符串
//         // char param[32];
//         char jsonBuf[128];
//         // sprintf(param, "{\"LightSwitch\":%d}", digitalRead(LED_B)); //我们把要上传的数据写在param里
//         // sprintf(param, "{\"temperature\":%.1f}", 36.6);
//         postMsgId += 1;
//         sprintf(jsonBuf, ALINK_BODY_FORMAT, postMsgId, ALINK_METHOD_PROP_POST, param);
//         // 再从mqtt客户端中发布post消息
//         if (mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf)) {
//             Serial.print("Post message to cloud: ");
//             Serial.println(jsonBuf);
//         } else {
//             Serial.println("Publish message to cloud failed!");
//         }
//     }
// }

// // tag [接收消息]收到消息回调,进行json处理并返回
// void callback(char *topic, byte *payload, unsigned int length)  {
//     if (strstr(topic, ALINK_TOPIC_PROP_SET)) {
//         Serial.println("收到下发的命令主题:");
//         Serial.println(topic);
//         Serial.println("下发的内容是:");
//         // 为payload添加一个结束附,防止Serial.println()读过了
//         payload[length] = '\0'; 
//         Serial.println((char *)payload);

//         // 接下来是收到的json字符串的解析
//         DynamicJsonDocument doc(100);
//         if ( deserializeJson(doc, payload) ) {
//             Serial.println("parse json failed");
//             return;
//         }
//         JsonObject setAlinkMsgObj = doc.as<JsonObject>();

//         // tag -----------------------后期删掉--------------------
//         serializeJsonPretty(setAlinkMsgObj, Serial);
//         Serial.println();
//         // 这里是一个点灯小逻辑
//         int lightSwitch = setAlinkMsgObj["params"]["LightSwitch"];
//         digitalWrite(LED, lightSwitch);
//         // ------------------------------------------------------

//         // return setAlinkMsgObj;
//     }
// }

// void serverSetup(void) {
//     // 初始化wifi
//     setupWiFi();
//     // 连接MQTT服务器
//     if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
//         Serial.println("MQTT服务器连接成功!");
//     };
//     // 订阅服务端set topic
//     mqttClient.subscribe(ALINK_TOPIC_PROP_SET);
//     // 绑定收到set主题时的回调
//     mqttClient.setCallback(callback);
// }

// void serverLoop(void) {
//     //检测有没有断线
//     if (!WiFi.isConnected()) {  // 检查WiFi连接
//         setupWiFi();
//     } else {
//         if (!mqttClient.connected()) { //检查mqtt连接
//             Serial.println("mqtt disconnected!Try reconnect now...");
//             Serial.println(mqttClient.state());
//             clientReconnect();
//         }
//     }
//     // mqtt客户端监听 【不会阻塞】
//     mqttClient.loop(); 
// }
