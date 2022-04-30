// #ifndef aliyunServer_h
// #define aliyunServer_h

// #define WIFI_SSID "HUAWEI nova4"  //wifi名
// #define WIFI_PASSWD "12345678"    //wifi密码

// #define PRODUCT_KEY "a1nKID4oRsp"                        // 产品ID
// #define DEVICE_NAME "ESP32"                              // 设备名
// #define DEVICE_SECRET "a687a3d6e96dc9272e90450841d8d564" // 设备key
// #define REGION_ID "cn-shanghai"                          // 区域

// // 服务端消息订阅Topic
// #define ALINK_TOPIC_PROP_SET "/a1nKID4oRsp/" DEVICE_NAME "/user/get"
// // 属性上报Topic
// #define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
// // 设备post上传数据要用到一个json字符串, 这个是拼接postJson用到的一个字符串
// #define ALINK_METHOD_PROP_POST "thing.event.property.post"
// // 这是post上传数据使用的模板
// #define ALINK_BODY_FORMAT "{\"id\":\"%u\",\"version\":\"1.0.0\",\"method\":\"%s\",\"params\":%s}"

// #define LED 2 // 定义LED灯的引脚

// /**
//  * @brief 发送消息
//  * 
//  * @param param json格式的字符串
//  */
// void mqttPublish(char *param);

// /**
//  * @brief 服务初始化，在setup中调用
//  * 
//  */
// void serverSetup(void);

// /**
//  * @brief 服务监听，loop中调用
//  * 
//  */
// void serverLoop(void);

// #endif