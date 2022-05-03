// #include <Arduino.h>
// #include "PubSubClient.h"
// #include "WiFi.h"
// #include <aliyun_mqtt.h>
// #include <ArduinoJson.h>
// #include "Ticker.h"
// #include <Adafruit_SSD1306.h>

// // 风扇引脚
// #define INA 13
// #define INB 12
// #define LED_B 2 // 定义LED灯的引脚

// #define WIFI_SSID "HUAWEI nova4"       //wifi名
// #define WIFI_PASSWD "12345678" //wifi密码

// #define PRODUCT_KEY "a1nKID4oRsp"                        //产品ID
// #define DEVICE_NAME "ESP32"                     //设备名
// #define DEVICE_SECRET "a687a3d6e96dc9272e90450841d8d564" //设备key
// #define REGION_ID "cn-shanghai"

// // 服务端消息订阅Topic
// #define ALINK_TOPIC_PROP_SET "/a1nKID4oRsp/" DEVICE_NAME "/user/get"
// // 属性上报Topic
// #define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
// // 设备post上传数据要用到一个json字符串, 这个是拼接postJson用到的一个字符串
// #define ALINK_METHOD_PROP_POST "thing.event.property.post"
// // 这是post上传数据使用的模板
// #define ALINK_BODY_FORMAT "{\"id\":\"%u\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s}"


// int rate = 0;      // 电机转速 0-255
// int direction = 0; // 电机转向
// int postMsgId = 0; //记录已经post了多少条
// Ticker tim1;       //这个定时器是为了每5秒上传一次数据


// /*------------------------------------------------------------------------------------------*/

// WiFiClient espClient;               //创建网络连接客户端
// PubSubClient mqttClient(espClient); //通过网络客户端连接创建mqtt连接客户端

// // 风扇处理
// void fanControl(bool flag, int data) {
// 	analogWrite(INA, flag ? 0    : data); // 0--255
// 	analogWrite(INB, flag ? data : 0);
// }


// //连接WIFI相关函数
// void setupWifi() {
// 	delay(10);
// 	Serial.println("连接WIFI");
// 	WiFi.begin(WIFI_SSID, WIFI_PASSWD);
// 	while (!WiFi.isConnected()) {
// 	Serial.print(".");
// 		delay(500);
// 	}
// 	Serial.println("OK");
// 	Serial.println("Wifi连接成功");
// }

// // 重连函数, 如果客户端断线,可以通过此函数重连
// void clientReconnect() {
// 	while (!mqttClient.connected()) { //再重连客户端
// 		Serial.println("reconnect MQTT...");
// 		if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
// 			Serial.println("connected");
// 		} else {
// 			Serial.println("failed");
// 			Serial.println(mqttClient.state());
// 			Serial.println("try again in 5 sec");
// 			delay(5000);
// 		}
// 	}
// }
// //mqtt发布post消息(上传数据)
// void mqttPublish() {
// 	if (mqttClient.connected()) {
// 		//先拼接出json字符串
// 		char param[32];
// 		char jsonBuf[128];
// 		// sprintf(param, "{\"LightSwitch\":%d}", digitalRead(LED_B)); //我们把要上传的数据写在param里
// 		sprintf(param, "{\"temperature\":%.1f}", 36.6); //我们把要上传的数据写在param里
// 		postMsgId += 1;
// 		sprintf(jsonBuf, ALINK_BODY_FORMAT, postMsgId, ALINK_METHOD_PROP_POST, param);
// 		//再从mqtt客户端中发布post消息
// 		if (mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf)) {
// 			Serial.print("Post message to cloud: ");
// 			Serial.println(jsonBuf);
// 		} else {
// 			Serial.println("Publish message to cloud failed!");
// 		}
// 	}
// }
// // 收到消息回调
// void callback(char *topic, byte *payload, unsigned int length)
// {
// 	if (strstr(topic, ALINK_TOPIC_PROP_SET)) {
// 		Serial.println("收到下发的命令主题:");
// 		Serial.println(topic);
// 		Serial.println("下发的内容是:");
// 		payload[length] = '\0'; //为payload添加一个结束附,防止Serial.println()读过了
// 		Serial.println((char *)payload);

// 		// 接下来是收到的json字符串的解析
// 		DynamicJsonDocument doc(100);
// 		DeserializationError error = deserializeJson(doc, payload);
// 		if (error)
// 		{
// 			Serial.println("parse json failed");
// 			return;
// 		}
// 		JsonObject setAlinkMsgObj = doc.as<JsonObject>();
// 		serializeJsonPretty(setAlinkMsgObj, Serial);
// 		Serial.println();

// 		// 风扇逻辑处理
// 		rate = setAlinkMsgObj["params"]["fanRate"];
// 		direction = setAlinkMsgObj["params"]["fanDirection"];
// 		Serial.print("rate: ");
// 		Serial.println(rate);
// 		Serial.print("direction: ");
// 		Serial.println(direction);

// 	}
// }

// //检测有没有断线
// void mqttCheck() {
// 	if (!WiFi.isConnected()) { // 判断WiFi是否连接
// 		setupWifi();
// 	}
// 	else { //如果WIFI连接了,
// 		if (!mqttClient.connected()) { //再看mqtt连接了没
// 			Serial.println("mqtt disconnected!Try reconnect now...");
// 			Serial.println(mqttClient.state());
// 			clientReconnect();
// 		}
// 	}
// }

// void setup() {
// 	pinMode(LED_B, OUTPUT);

// 	pinMode(INA, OUTPUT);
// 	pinMode(INB, OUTPUT);

// 	Serial.begin(115200);
// 	setupWifi();
// 	if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
// 		Serial.println("MQTT服务器连接成功!");
// 	}
// 	// ! 订阅Topic !!这是关键!!
// 	mqttClient.subscribe(ALINK_TOPIC_PROP_SET);
// 	mqttClient.setCallback(callback); // 绑定收到set主题时的回调(命令下发回调)
// 	// tim1.attach(10, mqttPublish);     // 启动每5秒发布一次消息
// }


// void loop() {
// 	mqttCheck();  // 检查连接
// 	mqttClient.loop();  // mqtt客户端监听 不会阻塞

// 	fanControl(direction, rate);
// }

//==================================================== oled ================================================
// #include <Arduino.h>
// #include <Wire.h>
 
// // 引入驱动OLED0.96所需的库
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
 
// #define SCREEN_WIDTH 128 // 设置OLED宽度,单位:像素
// #define SCREEN_HEIGHT 64 // 设置OLED高度,单位:像素
 
// // 自定义重置引脚,虽然教程未使用,但却是Adafruit_SSD1306库文件所必需的
// #define OLED_RESET 4
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// void setup() {
//     // Serial.begin(115200);
//     // 初始化Wire库
//     Wire.begin();
//     // 初始化OLED并设置其IIC地址为 0x3C => 0x02
//     display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
//     display.setCursor(0, 0);
//     display.print("TaichiMaker");
//     display.display();
// }
 
// void loop() {}

// ================================== u8g2 =============================
#include <Arduino.h>
#include <U8g2lib.h>

static const uint8_t ESP32_SCL = 5;
static const uint8_t ESP32_SDA = 18;

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset= U8X8_PIN_NONE*/ 4, ESP32_SCL, ESP32_SDA);
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,  /*SCL*/ ESP32_SCL,  /*SDA*/ ESP32_SDA, /*reset*/ U8X8_PIN_NONE);//构造
                                      

void setup(void) {
    u8g2.begin();
}

void loop(void) {
    u8g2.clearBuffer();                                       
    u8g2.setFont(u8g2_font_ncenB08_tr);       
    u8g2.drawStr(0, 10, "Hello World!");       
    u8g2.sendBuffer();                                       
    delay(1000);  
}

// ======================================= test ==============================================
// #include <Arduino.h>
// #include <U8g2lib.h>

// U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,  /*SCL*/  GPIO_NUM_5,  /*SDA*/  GPIO_NUM_18,   /*reset*/  U8X8_PIN_NONE);//构造
                                      

// typedef u8g2_uint_t u8g_uint_t;


// #define SECONDS 10
// uint8_t flip_color = 0;
// uint8_t draw_color = 1;

// void draw_set_screen(void) {
//   // graphic commands to redraw the complete screen should be placed here  
//   u8g2.setColorIndex(flip_color);
//   u8g2.drawBox( 0, 0, u8g2.getWidth(), u8g2.getHeight() );
// }

// void draw_clip_test(void) {
//   u8g_uint_t i, j, k;
//   char buf[3] = "AB";
//   k = 0;
//   u8g2.setColorIndex(draw_color);
//   u8g2.setFont(u8g2_font_6x10_tf);
  
//   for( i = 0; i  < 6; i++ ) {
//     for( j = 1; j  < 8; j++ ) {
//       u8g2.drawHLine(i-3, k, j);
//       u8g2.drawHLine(i-3+10, k, j);
      
//       u8g2.drawVLine(k+20, i-3, j);
//       u8g2.drawVLine(k+20, i-3+10, j);
      
//       k++;
//     }
//   }
//   u8g2.setFontDirection(0);
//   u8g2.drawStr(0-3, 50, buf);
//   u8g2.setFontDirection(2);
//   u8g2.drawStr(0+3, 50, buf);
  
//   u8g2.setFontDirection(0);
//   u8g2.drawStr(u8g2.getWidth()-3, 40, buf);
//   u8g2.setFontDirection(2);
//   u8g2.drawStr(u8g2.getWidth()+3, 40, buf);

//   u8g2.setFontDirection(1);
//   u8g2.drawStr(u8g2.getWidth()-10, 0-3, buf);
//   u8g2.setFontDirection(3);
//   u8g2.drawStr(u8g2.getWidth()-10, 3, buf);

//   u8g2.setFontDirection(1);
//   u8g2.drawStr(u8g2.getWidth()-20, u8g2.getHeight()-3, buf);
//   u8g2.setFontDirection(3);
//   u8g2.drawStr(u8g2.getWidth()-20, u8g2.getHeight()+3, buf);

//   u8g2.setFontDirection(0);

// }

// void draw_char(void) {
//   char buf[2] = "@";
//   u8g_uint_t i, j;
//   // graphic commands to redraw the complete screen should be placed here  
//   u8g2.setColorIndex(draw_color);
//   u8g2.setFont(u8g2_font_6x10_tf);
//   j = 8;
//   for(;;) {
//     i = 0;
//     for(;;) {
//       u8g2.drawStr( i, j, buf);
//       i += 8;
//       if ( i > u8g2.getWidth() )
//         break;
//     }
//     j += 8;
//     if ( j > u8g2.getHeight() )
//       break;
//   }
  
// }

// void draw_pixel(void) {
//   u8g_uint_t x, y, w2, h2;
//   u8g2.setColorIndex(draw_color);
//   w2 = u8g2.getWidth();
//   h2 = u8g2.getHeight();
//   w2 /= 2;
//   h2 /= 2;
//   for( y = 0; y < h2; y++ ) {
//     for( x = 0; x < w2; x++ ) {
//       if ( (x + y) & 1 ) {
//         u8g2.drawPixel(x,y);
//         u8g2.drawPixel(x,y+h2);
//         u8g2.drawPixel(x+w2,y);
//         u8g2.drawPixel(x+w2,y+h2);
//       }
//     }
//   }
// }

// void draw_line(void) {
//   u8g2.setColorIndex(draw_color);
//   u8g2.drawLine(0,0, u8g2.getWidth()-1, u8g2.getHeight()-1);
// }

// // returns unadjusted FPS
// uint16_t execute_with_fps(void (*draw_fn)(void)) {
//   uint16_t FPS10 = 0;
//   uint32_t time;
  
//   time = millis() + SECONDS*1000;
  
//   // picture loop
//   do {
//     u8g2.clearBuffer();
//     draw_fn();
//     u8g2.sendBuffer();
//     FPS10++;
//     flip_color = flip_color ^ 1;
//   } while( millis() < time );
//   return FPS10;  
// }

// const char *convert_FPS(uint16_t fps) {
//   static char buf[6];
//   strcpy(buf, u8g2_u8toa( (uint8_t)(fps/10), 3));
//   buf[3] =  '.';
//   buf[4] = (fps % 10) + '0';
//   buf[5] = '\0';
//   return buf;
// }

// void show_result(const char *s, uint16_t fps) {
//   // assign default color value
//   u8g2.setColorIndex(draw_color);
//   u8g2.setFont(u8g2_font_8x13B_tf);
//   u8g2.clearBuffer();
//   u8g2.drawStr(0,12, s);
//   u8g2.drawStr(0,24, convert_FPS(fps));
//   u8g2.sendBuffer();
// }

// void setup(void) {


//   u8g2.begin();

//   draw_color = 1;         // pixel on
  
//   //u8g2.setBusClock(2000000);
// }

// void loop(void) {
//   uint16_t fps;
//   fps = execute_with_fps(draw_clip_test);
//   show_result("draw clip test", fps);
//   delay(5000);
//   fps = execute_with_fps(draw_set_screen);
//   show_result("clear screen", fps);
//   delay(5000);
//   fps = execute_with_fps(draw_char);
//   show_result("draw @", fps);
//   delay(5000);  
//   fps = execute_with_fps(draw_pixel);
//   show_result("draw pixel", fps);
//   delay(5000);
//   fps = execute_with_fps(draw_line);
//   show_result("draw line", fps);
//   delay(5000);
// }
