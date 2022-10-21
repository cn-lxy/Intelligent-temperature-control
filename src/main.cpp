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

/*---- 函数定义 ------- */
void mqttPublish(void);

/*------------------------------------------ GPIO --------------------------------------------------*/
// TAG
// // GPIO DS18B20 
// #define DS18B20_PIN 4

// // GPIO OLED IIC引脚
// static const uint8_t ESP32_SCL = 5;
// static const uint8_t ESP32_SDA = 18;

// // GPIO 风扇引脚
// #define INA_PIN 13
// #define INB_PIN 12

// // GPIO 制冷和制热 
// #define HOT_PIN  14
// #define COLD_PIN 27

// TAG version-2
// GPIO DS18B20 
#define DS18B20_PIN 32

// GPIO OLED IIC引脚
static const uint8_t ESP32_SCL = 22;
static const uint8_t ESP32_SDA = 21;

// GPIO 风扇引脚
#define INA_PIN 13
#define INB_PIN 12

// GPIO 制冷和制热 
#define HOT_PIN  14
#define COLD_PIN 27


// TAG 温度加减按钮
#define BTN_ADD 26
#define BTN_SUB 25

// Board LED
#define LED 2

/*------------------------------------------ WiFi & MQTT --------------------------------------------*/
#define WIFI_SSID "DESKTOP-VADIOP2 9584"    //wifi名
#define WIFI_PASSWD "12345678"  			//wifi密码

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
#define ALINK_BODY_FORMAT "{\"id\":\"%u\",\"version\":\"1.0.0\",\"method\":\"%s\",\"params\":%s}"

/*----------------------------------------- 全局变量 -------------------------------------------------*/
volatile int windSpeed = 0;      	// 电机转速等级 0: 关闭; 1: 低速; 2: 中速; 3: 高速;
volatile int direction = 0; 		// 电机转向
int postMsgId = 0; 			  		// 记录已经post了多少条
Ticker timer;       		  		// 这个定时器是为了每5秒上传一次数据
volatile int envTemperature = 0;    // 环境温度
volatile int setTemperature  = 0;   // 设置温度
volatile int mode = 0;      	    // 模式 => [0:制冷, 1:加热]

/*------------------------------------------- object -----------------------------------------------*/
WiFiClient espClient;               //创建网络连接客户端
PubSubClient mqttClient(espClient); //通过网络客户端连接创建mqtt连接客户端

U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,  /*SCL*/ ESP32_SCL,  /*SDA*/ ESP32_SDA, /*reset*/ U8X8_PIN_NONE);//构造

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
/*-------------------------------------------------------------------------------------------------*/

// 模式切换
void modeChange() {
	if (mode == 0) {
		digitalWrite(COLD_PIN, 1);
		digitalWrite(HOT_PIN, 0);
	} else if (mode == 1) {
		digitalWrite(COLD_PIN, 0);
		digitalWrite(HOT_PIN, 1);
	}
}

// TAG 按键处理 [风速，制热制冷]
void keysHandler() {
	// TAG 制热
	// if (digitalRead(HOT_PIN) == 1 && digitalRead(COLD_PIN) == 0) {
	// 	delay(10);
	// 	if (digitalRead(HOT_PIN) == 1 && digitalRead(COLD_PIN) == 0)
	// 		mode = 1;
	// } else if (digitalRead(HOT_PIN) == 0 && digitalRead(COLD_PIN) == 1) {
	// 制冷
	// 	delay(10);
	// 	if (digitalRead(HOT_PIN) == 0 && digitalRead(COLD_PIN) == 1)
	// 		mode = 0;
	// }

	// TAG风速等级切换
	// if (digitalRead(GRADE_LOW) == 1 && digitalRead(GRADE_MIDDLE) == 0 && digitalRead(GRADE_LOW) == 0) {
	// 	delay(10);
	// 	if (digitalRead(GRADE_LOW) == 1 && digitalRead(GRADE_LOW) == 0 && digitalRead(GRADE_LOW) == 0)
	// 		windSpeed = 1;
	// } else if (digitalRead(GRADE_LOW) == 0 && digitalRead(GRADE_LOW) == 1 && digitalRead(GRADE_LOW) == 0) {
	// 	delay(10);
	// 	if (digitalRead(GRADE_LOW) == 0 && digitalRead(GRADE_LOW) == 1 && digitalRead(GRADE_LOW) == 0)
	// 		windSpeed = 2;
	// } else if (digitalRead(GRADE_LOW) == 0 && digitalRead(GRADE_LOW) == 0 && digitalRead(GRADE_LOW) == 1) {
	// 	delay(10);
	// 	if (digitalRead(GRADE_LOW) == 0 && digitalRead(GRADE_LOW) == 0 && digitalRead(GRADE_LOW) == 1)
	// 		windSpeed = 3;
	// }

	// TAG 温度加减判断 低电平触发
	if (digitalRead(BTN_ADD) == 0 && digitalRead(BTN_SUB) == 1) {
		delay(10);
		if (digitalRead(BTN_ADD) == 0 && digitalRead(BTN_SUB) == 1) {
			if (setTemperature < 40)
				setTemperature++;
			mqttPublish();
		}
	} else if (digitalRead(BTN_ADD) == 1 && digitalRead(BTN_SUB) == 0) {
		delay(10);
		if (digitalRead(BTN_ADD) == 1 && digitalRead(BTN_SUB) == 0) {
			if (setTemperature > 0 )
				setTemperature--;
			mqttPublish();
		}
	}
}

// 风扇控制
void fanControl() {
	int rate = 0;
	Serial.printf("windSpeed: %d\n", windSpeed);
	if (windSpeed == 1) {
		rate = 400;
	} else if (windSpeed == 2) {
		rate = 700;
	} else if (windSpeed == 3) {
		rate = 1024;
	} else {
		rate = 0;
	}
	
	// 当温度达到设定温度时会自动关闭风扇
	// if ( (mode == 0 && envTemperature <= setTemperature) || (mode == 1 && envTemperature >= setTemperature) ) {
	// 	rate = 0;
	// }

	// BUG `analogWrite` function is not implemented is ESP32 lib.
	// analogWrite(INA_PIN, direction ? 0    : rate); // 0--255
	// analogWrite(INB_PIN, direction ? rate : 0);
	
	ledcWrite(1, direction ? 0    : rate);
	ledcWrite(2, direction ? rate : 0);
}

// 屏幕打印信息
void oledLog(String s) {
	u8g2.clearBuffer();                                       
    u8g2.setFont(u8g2_font_ncenB08_tr);      
    u8g2.setColorIndex(3);
	u8g2.drawStr(0, 10, s.c_str());
	u8g2.sendBuffer();
}

// 屏幕显示
void oledDisplay() {
    u8g2.clearBuffer();                                       
    u8g2.setFont(u8g2_font_ncenB08_tr);      
    u8g2.setColorIndex(2); 
    char info1[32];
    char info2[32];
    char info3[32];
    char info4[32];
    sprintf(info1, "setTemperature: %d", setTemperature);
    sprintf(info2, "readTemperature: %d", envTemperature);
	if (windSpeed == 1) {
		sprintf(info3, "grade: %s", "low");
	} else if (windSpeed == 2) {
		sprintf(info3, "grade: %s", "center");
	} else if (windSpeed == 3) {
		sprintf(info3, "grade: %s", "hight");
	} else {
		sprintf(info3, "grade: %s", "off");
	}
    sprintf(info4, "mode: %s", mode == 0 ? "cold" : (mode == 1) ? "hot" : "");

    u8g2.drawStr(0, 10, info1);       
    u8g2.drawStr(0, 20, info2);       
    u8g2.drawStr(0, 30, info3);       
    u8g2.drawStr(0, 40, info4);       

    u8g2.sendBuffer();                                       
}

//连接WIFI相关函数
void setupWifi() {
	delay(10);
	Serial.println("连接WIFI");
	WiFi.begin(WIFI_SSID, WIFI_PASSWD);
	while (!WiFi.isConnected()) {
		Serial.print(".");
			delay(500);
	}
	Serial.println("OK");
	Serial.println("Wifi连接成功");
}

// 重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect() {
	while (!mqttClient.connected()) { //再重连客户端
		Serial.println("reconnect MQTT...");
		if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
			Serial.println("connected");
		} else {
			Serial.println("failed");
			Serial.println(mqttClient.state());
			Serial.println("try again in 5 sec");
			delay(5000);
		}
	}
}

// mqtt发布post消息(上传数据)
void mqttPublish() {
	if (mqttClient.connected()) {
		//先拼接出json字符串
		char param[128];
		char jsonBuf[256];
		// 云端属性采用首字母大写驼峰命名法
		sprintf(param, "{\"EnvTemperature\": %d, \"WindSpeed\": %d, \"SetTemperature\": %d}", envTemperature, windSpeed, setTemperature); //我们把要上传的数据写在param里
		postMsgId += 1;
		sprintf(jsonBuf, ALINK_BODY_FORMAT, postMsgId, ALINK_METHOD_PROP_POST, param);
		//再从mqtt客户端中发布post消息
		if (mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf)) {
			Serial.print("Post message to cloud: ");
			Serial.println(jsonBuf);
		} else {
			Serial.println("Publish message to cloud failed!");
		}
		digitalWrite(LED, HIGH);
		delay(50);
		digitalWrite(LED, LOW);
	}
}

// 收到消息回调
void callback(char *topic, byte *payload, unsigned int length)
{
	if (strstr(topic, ALINK_TOPIC_PROP_SET)) {
		Serial.println("收到下发的命令主题:");
		Serial.println(topic);
		Serial.println("下发的内容是:");
		payload[length] = '\0'; //为payload添加一个结束附,防止Serial.println()读过了
		Serial.println((char *)payload);

		// 接下来是收到的json字符串的解析
		DynamicJsonDocument doc(150);
		DeserializationError error = deserializeJson(doc, payload);
		if (error)
		{
			Serial.println("parse json failed");
            Serial.println(error.c_str());
			return;
		}
		JsonObject setAlinkMsgObj = doc.as<JsonObject>();
		serializeJsonPretty(setAlinkMsgObj, Serial);
		Serial.println();

		// 风扇逻辑处理
		windSpeed      = setAlinkMsgObj["params"]["fanRate"];
		direction      = setAlinkMsgObj["params"]["fanDirection"];
        setTemperature = setAlinkMsgObj["params"]["temperature"];
		mode           = setAlinkMsgObj["params"]["mode"];
		Serial.print("windSpeed: ");
		Serial.println(windSpeed);
		Serial.print("direction: ");
		Serial.println(direction);
		Serial.print("setTemperature: ");
		Serial.println(setTemperature);
		Serial.print("mode: ");
		Serial.println(mode);
	}
}

//检测有没有断线
void mqttCheck() {
	if (!WiFi.isConnected()) { // 判断WiFi是否连接
		setupWifi();
	}
	else { //如果WIFI连接了,
		if (!mqttClient.connected()) { //再看mqtt连接了没
			Serial.println("mqtt disconnected!Try reconnect now...");
			Serial.println(mqttClient.state());
			clientReconnect();
		}
	}
}

// 获取DS18B20温度
void getTemperature() {
	sensors.requestTemperatures(); 
	float tempC = sensors.getTempCByIndex(0);
	if(tempC != DEVICE_DISCONNECTED_C) {
		Serial.print("Temperature for the device 1 (index 0) is: ");
		Serial.println(tempC);
	} else {
	Serial.println("Error: Could not read temperature data");
	}
	envTemperature = int(tempC);
	// Serial.print(temperatureC);
	// Serial.println("ºC");
}

void log() {
	Serial.printf("grade: %d\n", windSpeed);
	Serial.printf("envTemperature: %d\n", envTemperature);
	Serial.printf("setTemperature: %d\n", setTemperature);
	Serial.printf("mode: %d\n", mode);
}

void setup() {
    u8g2.begin();

	oledLog("init pin");

	Serial.begin(115200);
	pinMode(INA_PIN, OUTPUT);
	pinMode(INB_PIN, OUTPUT);
	pinMode(DS18B20_PIN, OUTPUT);
	pinMode(HOT_PIN, OUTPUT);
	pinMode(COLD_PIN, OUTPUT);
	pinMode(BTN_ADD, INPUT_PULLUP);
	pinMode(BTN_SUB, INPUT_PULLUP);
	pinMode(LED, OUTPUT);

	ledcSetup(1, 1000, 10);
	ledcAttachPin(INA_PIN, 1);
    ledcSetup(2, 1000, 10);
	ledcAttachPin(INB_PIN, 2);

	oledLog("init DS18B20 sensor");
	
	// Start the DS18B20 sensor
	sensors.begin();

	oledLog("init wifi");

	setupWifi();
	if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
		Serial.println("MQTT服务器连接成功!");
	}

	oledLog("init mqtt");

	mqttClient.subscribe(ALINK_TOPIC_PROP_SET); // ! 订阅Topic !!这是关键!!
	mqttClient.setCallback(callback);  			// 绑定收到set主题时的回调(命令下发回调)
	timer.attach(10, mqttPublish);     			// 启动每5秒发布一次消息

	oledLog("all init success!");
	delay(500);
}


void loop() {
	mqttCheck();  // 检查连接
	mqttClient.loop();  // mqtt客户端监听 不会阻塞

	getTemperature();
	keysHandler();
	fanControl();
	modeChange();
    oledDisplay();
	log();
}
