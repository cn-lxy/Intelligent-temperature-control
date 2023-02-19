#include "main.h"
#include <Arduino.h>

//! 初始化函数
void setup()
{
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
	if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
	{
		Serial.println("MQTT服务器连接成功!");
	}

	oledLog("init mqtt");

	mqttClient.subscribe(ALINK_TOPIC_PROP_SET); // ! 订阅Topic !!这是关键!!
	mqttClient.setCallback(callback);			// 绑定收到set主题时的回调(命令下发回调)

	// 创建 Mutex
	xMutexInner = xSemaphoreCreateMutex();
	if (xMutexInner == NULL)
	{
		Serial.println("No Enough RAM, unable to create `Semaphore.`");
	}
	xMutexOuter = xSemaphoreCreateMutex();
	if (xMutexOuter == NULL)
	{
		Serial.println("No Enough RAM, unable to create `Semaphore.`");
	}

	// 注册 task
	if (xTaskCreatePinnedToCore(sensorGetTask, "sensorGetTask", 1024 * 5, NULL, 1, NULL, 1) == pdPASS)
		Serial.println("sensorGetTask 创建成功");
	if (xTaskCreatePinnedToCore(displayTask, "displayTask", 1024 * 5, NULL, 1, NULL, 1) == pdPASS)
		Serial.println("displayTask 创建成功");
	if (xTaskCreatePinnedToCore(netCheckTask, "netCheckTask", 1024 * 5, NULL, 1, NULL, 1) == pdPASS)
		Serial.println("netCheckTask 创建成功");
	if (xTaskCreatePinnedToCore(driverTask, "driverTask", 1024 * 5, NULL, 1, NULL, 1) == pdPASS)
		Serial.println("driverTask 创建成功");
	if (xTaskCreatePinnedToCore(uploadData, "uploadData", 1024 * 5, NULL, 1, NULL, 1) == pdPASS)
		Serial.println("uploadData 创建成功");
	if (xTaskCreatePinnedToCore(logTask, "logTask", 1024 * 5, NULL, 1, NULL, 1) == pdPASS)
		Serial.println("logTask 创建成功");

	oledLog("all init success!");
	delay(500);
}

//! 循环函数
void loop() {}

// 模式切换
void modeChange()
{
	if (aliData.mode == 0)
	{
		digitalWrite(COLD_PIN, 1);
		digitalWrite(HOT_PIN, 0);
	}
	else if (aliData.mode == 1)
	{
		digitalWrite(COLD_PIN, 0);
		digitalWrite(HOT_PIN, 1);
	}
}

// 按键处理 [风速，制热制冷]
void keysHandler()
{
	// 温度加减判断 低电平触发
	if (digitalRead(BTN_ADD) == 0 && digitalRead(BTN_SUB) == 1)
	{
		delay(10);
		if (digitalRead(BTN_ADD) == 0 && digitalRead(BTN_SUB) == 1)
		{
			if (aliData.setTemperature < 40)
				aliData.setTemperature++;
		}
	}
	else if (digitalRead(BTN_ADD) == 1 && digitalRead(BTN_SUB) == 0)
	{
		delay(10);
		if (digitalRead(BTN_ADD) == 1 && digitalRead(BTN_SUB) == 0)
		{
			if (aliData.setTemperature > 0)
				aliData.setTemperature--;
		}
	}
}

// 风扇控制
void fanControl()
{
	int rate = 0;
	Serial.printf("windSpeed: %d\n", aliData.windSpeed);
	if (aliData.windSpeed == 1)
	{
		rate = 400;
	}
	else if (aliData.windSpeed == 2)
	{
		rate = 700;
	}
	else if (aliData.windSpeed == 3)
	{
		rate = 1024;
	}
	else
	{
		rate = 0;
	}

	// 当温度达到设定温度时会自动关闭风扇
	if ((aliData.mode == 0 && innerData.envTemperature <= aliData.setTemperature) || (aliData.mode == 1 && innerData.envTemperature >= aliData.setTemperature))
	{
		rate = 0;
	}

	// BUG `analogWrite` function is not implemented is ESP32 lib.
	// analogWrite(INA_PIN, direction ? 0    : rate); // 0--255
	// analogWrite(INB_PIN, direction ? rate : 0);

	ledcWrite(1, rate);
	ledcWrite(2, 0);
}

// 屏幕打印信息
void oledLog(String s)
{
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.setColorIndex(3);
	u8g2.drawStr(0, 10, s.c_str());
	u8g2.sendBuffer();
}

// 连接WIFI相关函数
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

// 重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect()
{
	while (!mqttClient.connected())
	{
		// 再重连客户端
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
			vTaskDelay(5000);
		}
	}
}

// 收到消息回调
void callback(char *topic, byte *payload, unsigned int length)
{
	if (strstr(topic, ALINK_TOPIC_PROP_SET) && xSemaphoreTake(xMutexOuter, timeout) == pdPASS)
	{
		Serial.println("收到下发的命令主题:");
		Serial.println(topic);
		Serial.println("下发的内容是:");
		payload[length] = '\0'; // 为payload添加一个结束附,防止Serial.println()读过了
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

		// 解析数据
		aliData.windSpeed = setAlinkMsgObj["params"]["fanRate"];
		aliData.setTemperature = setAlinkMsgObj["params"]["temperature"];
		aliData.mode = setAlinkMsgObj["params"]["mode"];

		// 释放锁
		xSemaphoreGive(xMutexOuter);

		Serial.print("windSpeed: ");
		Serial.println(aliData.windSpeed);
		Serial.print("setTemperature: ");
		Serial.println(aliData.setTemperature);
		Serial.print("mode: ");
		Serial.println(aliData.mode);
	}
}

// 检测有没有断线
void mqttCheck()
{
	if (!WiFi.isConnected())
	{ // 判断WiFi是否连接
		setupWifi();
	}
	else
	{ // 如果WIFI连接了,
		if (!mqttClient.connected())
		{ // 再看mqtt连接了没
			Serial.println("mqtt disconnected!Try reconnect now...");
			Serial.println(mqttClient.state());
			clientReconnect();
		}
	}
}

// [task] 获取传感器数据
void sensorGetTask(void *ptParams)
{
	const TickType_t xFrequency = 100; // 采集频率 `100 ticks` = 100ms
	TickType_t lastSleepTick = xTaskGetTickCount();

	while (true)
	{
		// 获取锁
		if (xSemaphoreTake(xMutexInner, timeout) == pdPASS)
		{
			sensors.requestTemperatures();
			float tempC = sensors.getTempCByIndex(0);
			if (tempC != DEVICE_DISCONNECTED_C)
			{
				Serial.print("Temperature for the device 1 (index 0) is: ");
				Serial.println(tempC);
			}
			else
			{
				Serial.println("Error: Could not read temperature data");
			}
			innerData.envTemperature = int(tempC);
		}

		// Serial.print(temperatureC);
		// Serial.println("ºC");
		vTaskDelayUntil(&lastSleepTick, xFrequency);
	}
}

// [task] 屏幕显示
void displayTask(void *ptParams)
{
	char info1[32];
	char info2[32];
	char info3[32];
	char info4[32];
	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;

	while (true)
	{
		// 获取两个锁
		if (xSemaphoreTake(xMutexInner, timeout) == pdPASS && xSemaphoreTake(xMutexOuter, timeout) == pdPASS)
		{
			// 清除
			u8g2.clearBuffer();
			// 字体，字体颜色设置
			u8g2.setFont(u8g2_font_ncenB08_tr);
			u8g2.setColorIndex(2);
			// 显示数据准备
			sprintf(info1, "setTemperature: %d", aliData.setTemperature);
			sprintf(info2, "readTemperature: %d", innerData.envTemperature);
			if (aliData.windSpeed == 1)
			{
				sprintf(info3, "grade: %s", "low");
			}
			else if (aliData.windSpeed == 2)
			{
				sprintf(info3, "grade: %s", "center");
			}
			else if (aliData.windSpeed == 3)
			{
				sprintf(info3, "grade: %s", "hight");
			}
			else
			{
				sprintf(info3, "grade: %s", "off");
			}
			sprintf(info4, "mode: %s", aliData.mode == 0 ? "cold" : (aliData.mode == 1) ? "hot"
																						: "");

			// 释放锁
			xSemaphoreGive(xMutexInner);
			xSemaphoreGive(xMutexOuter);

			// 显示
			u8g2.drawStr(0, 10, info1);
			u8g2.drawStr(0, 20, info2);
			u8g2.drawStr(0, 30, info3);
			u8g2.drawStr(0, 40, info4);
			u8g2.sendBuffer();
		}
		vTaskDelay(xDelay);
	}
}

// [task] 网络检查
void netCheckTask(void *ptParams)
{
	const TickType_t xFrequency = 1000; // 1000 ticks = 1000ms
	TickType_t xLastSleepTick = xTaskGetTickCount();

	while (true)
	{
		mqttCheck();	   // 检查mqtt连接
		mqttClient.loop(); // mqtt客户端监听，不会阻塞
		vTaskDelayUntil(&xLastSleepTick, xFrequency);
	}
}

// [task] 外设控制
void driverTask(void *ptParams)
{
	while (true)
	{
		if (xSemaphoreTake(xMutexInner, timeout) == pdPASS && xSemaphoreTake(xMutexOuter, timeout) == pdPASS)
		{
			keysHandler();
			modeChange();
			fanControl();

			// 释放锁
			xSemaphoreGive(xMutexInner);
			xSemaphoreGive(xMutexOuter);
		}
	}
}

// [task] 数据上传
void uploadData(void *ptParams)
{
	char param[128];
	char jsonBuf[256];

	const TickType_t xFrequency = 5000; // 5000 ticks = 5000ms
	TickType_t xLastSleepTick = xTaskGetTickCount();

	while (true)
	{
		if (xSemaphoreTake(xMutexInner, timeout) == pdPASS && xSemaphoreTake(xMutexOuter, timeout) == pdPASS)
		{
			if (mqttClient.connected())
			{
				// 先拼接出json字符串
				// 云端属性采用首字母大写驼峰命名法
				// 我们把要上传的数据写在param里
				sprintf(param, "{\"EnvTemperature\": %d, \"WindSpeed\": %d, \"SetTemperature\": %d}", innerData.envTemperature, aliData.windSpeed, aliData.setTemperature);

				// 释放锁
				xSemaphoreGive(xMutexInner);
				xSemaphoreGive(xMutexOuter);

				postMsgId += 1;
				sprintf(jsonBuf, ALINK_BODY_FORMAT, postMsgId, ALINK_METHOD_PROP_POST, param);
				// 再从mqtt客户端中发布post消息
				if (mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf))
				{
					Serial.print("Post message to cloud: ");
					Serial.println(jsonBuf);
				}
				else
				{
					Serial.println("Publish message to cloud failed!");
				}
				digitalWrite(LED, HIGH);
				delay(50);
				digitalWrite(LED, LOW);

				vTaskDelayUntil(&xLastSleepTick, xFrequency);
			}
			else
			{
				clientReconnect();
			}
		}
	}
}

// [task] 打印日志
void logTask(void *ptParams)
{
	const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

	while (true)
	{
		Serial.printf("grade: %d\n", aliData.windSpeed);
		Serial.printf("envTemperature: %d\n", innerData.envTemperature);
		Serial.printf("setTemperature: %d\n", aliData.setTemperature);
		Serial.printf("mode: %d\n", aliData.mode);

		vTaskDelay(xDelay);
	}
}