# 涂鸦mesh BLE

主要硬件:mesh BLE 模块, sht30, 主控HC32L110,lcd12864屏幕

1. 干簧管,霍尔开关,振动开关检测门窗,
2. sht30检测温湿度上报,lcd1286显示温湿度,SPI Flash存储字库
3. 空气质量检测,
4. 红外遥控功能,远程遥控空调电视等
5. 太阳能板升压到5V给锂电池充电,锂电池LDO降压到3.3V系统使用
6. 华大MCU HC32L110 低功耗 做主控32Kfalsh 4Kram,
7. 低功耗BLE Mesh 传输数据,多点Mesh组网传输数据,相对WIFI功耗低

硬件PCB:https://oshwhub.com/miwang186/lan-yamesh-wen-shi-du

涂鸦模块:https://developer.tuya.com/cn/docs/iot/device-development/module/ble-module/tybt-series-module/bletybt5module?id=K989rhpdqls8g

软件程序:https://gitee.com/miwang186/tuya_mesh_ble

更新日志:
时间:20210307

**已知bug原理图错误**
>>  
	1. ME6214C45M5G  ldo降压 CE脚应该拉高使能,型号选择错误 应选ME6214C33M5G
	2. TPS61221DCKR  升压 应选择6120可以调压输出
	3. TPS61220DCKR  升压电感L1 原理图错误
	4. KEY_ADC分压错误 前端缺少电阻分压,ADC无法区分触发源

**已完成功能:**
>>  
	1. sht30 读取温湿度
	2. mesh BLE模块协议上传温湿度,APP连接查看温湿度
	3. 采集电池电压 电量上传
	4. 霍尔元件干弹簧 门窗检测,状态上传