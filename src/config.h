//设备三元组外加一个区域ID
#define PRODUCT_KEY "a1nKID4oRsp"
#define DEVICE_NAME "ESP32"
#define DEVICE_SECRET "a687a3d6e96dc9272e90450841d8d564"
#define REGION_ID "cn-shanghai"

/*需要上报和订阅的两个TOPIC*/
const char* subTopic = "/a1nKID4oRsp/ESP32/user/setAndget";//****set
const char* pubTopic = "/a1nKID4oRsp/ESP32/user/ESP32";//******post

//无线网络名称及密码
#define WIFI_SSID "HUAWEI nova4"
#define WIFI_PASSWD "12345678"