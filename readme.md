# 基于ESP32的智能温控器

## 设计文档

1. 作品简介
   本作品是为了远程控制中央空调风机盘管的通断阀，风机高中低转速。夏天天气温度比较高，早上到办公室打开办公室空调，室内温度需要很长时间才能降下来。用户可以通过智能温控器把中央空调提前打开。有时候下班时忘记关闭空调，可以通过手机软件关闭，或者在温控器设置定时关闭。办公室、厂房空间比较大，温控器采样温度与出口温度不一致，用户可以通过手机软件设定温度和风速进行微调。为了防止有人故意把空调温度设置过低或者过高，可以通过手机APP将面板进行锁定。
2. 设计原理
   本作品使用ESP32作为设备主控，使用微信小程序作为客户端。ESP32采集DS18B20温度传感器数据，获取当前环境温度，并将当前环境温度通过WiFi网络上传至阿里云物联网平台，微信小程序可以获取到设备上传的温度数据。微信小程序也可以给设备下发指令，让其执行任务，如：调节风速，切换模式等。
   本作品使用ESP32作为设备主控。ESP32采集DS18B20温度传感器数据，获取当前环境温度，并将温度显示在屏幕上，用户可以通过按键调节预定温度，并将预定的温度显示在屏幕上；当环境温度达到预定温度时，ESP32控制空调停止工作。
3. 功能说明

   1. 模式切换: 制冷 制热
    > 用户通过微信小程序远程控制设备的模式；
    > 并将结果显示在设备的屏幕上；

   2. 预定温度:
   > 用户可以按设备上的按钮来加减预定温度
   > 或者也可以通过微信小程序设置；
   > 并将结果显示在设备的屏幕上；

   3. 风速控制: 关 一级 二级 三级
   > 通过微信小程序远程设置设备的风速；
   > 并将结果显示在设备的屏幕上；

   4. 获取环境温度
   > 设备获取环境温度，将其显示在设备屏幕上，并将该数据上传至阿里云互联网平台

## 程序执行流程

程序首先执行setup初始化函数，在setup函数中对屏幕显示、串口、GPIO模式、DS18B20温度传感器、WiFi、MQTT进行初始化，然后设置定时消息发布，绑定云端消息回调。然后循环执行loop函数，在loop函数中，首先执行MQTT连接检查函数，若设备掉线，则重新连接，若设备连接正常，则往下执行依次执行，获取DS18B20传感器数据、按键扫描、风速控制、模式切换检查、屏幕显示、日志打印。

