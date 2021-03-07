/******************************************************************************
 * Include files
 ******************************************************************************/
#include "lcd12864.h"
#include "sht30.h"
#include "mesh_ble.h"
#include "switch_adc.h"
#include "common.h"
#include "bsp.h"

uint32_t tick_time_100ms = 0;
bool timer_collection_data_flag = false;
bool timer_upload_data_flag = false;

static void Time0_Int_Callback()
{
    if (TRUE == Bt_GetIntFlag(TIM0))
    {
		tick_time_100ms++;
		
		if((tick_time_100ms & 0x0f) == 0)/*1.5秒*/
			timer_collection_data_flag = true;
		
		if((tick_time_100ms & 0x3f) == 0)/*6秒*/
			timer_upload_data_flag = true;		
		
		//printf_log(0, "tick %d \n",tick_time_100ms);
		Bt_ClearIntFlag(TIM0);		
    }
}

static void Timer0_Config(uint16_t time_arr)
{
    stc_bt_config_t   stcConfig;
	
	Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);
    stcConfig.pfnTim0Cb = Time0_Int_Callback;
        
    stcConfig.enPRS   = BtPCLKDiv256;
    stcConfig.enCT    = BtTimer;
    stcConfig.enMD    = BtMode2;
    //Bt初始化
	Bt_Init(TIM0, &stcConfig);
    
    //TIM1中断使能
    Bt_ClearIntFlag(TIM0);
    Bt_EnableIrq(TIM0);
    EnableNvic(TIM0_IRQn, 3, TRUE);
	
	time_arr = 0x10000 - time_arr;
	
    //设置重载值和计数值，启动计数
    Bt_ARRSet(TIM0, time_arr);
    Bt_Cnt16Set(TIM0, time_arr);
    Bt_Run(TIM0);
}

static int current_temperature = 0, current_humidity = 0, battery_percentage = 0;
bool current_switch_status = false;

void printf_sht30_data()
{
	float temperature = 0,humidity = 0;
	char printf_buff[64];
	
	if(read_sht30_temp(&temperature,&humidity))
	{
		sprintf(printf_buff,"t:%4.2f h:%4.2f\n",temperature,humidity);
		printf_log(0,printf_buff);	
		current_temperature = temperature * 10;
		current_humidity = humidity;
	}
}
//const uint16_t voltage_quantity_max_tab[] = {420,406,398,392,387,382,379,377,374,368,345,300};
//const uint16_t voltage_quantity_min_tab[] = {420,397,387,379,373,368,365,362,358,351,342,300};

void printf_bat_voltage()
{
	uint16_t bat_voltage = (((get_bat_adc_value() / 4096.0) * 3.3 ) / 0.769) * 100 + 5;
	/*0.796 bat/adc_voltage  5 误差补偿*/
	
	printf_log(0,"bat_voltage %d.%dV\n", bat_voltage/100,bat_voltage%100);
	battery_percentage = bat_voltage - 320; 
}

bool printf_switch_status()
{
	static bool last_status = false;
	current_switch_status = !get_switch_status();
	
	if(last_status != current_switch_status)
	{
		printf_log(0, "switch status %d \n",current_switch_status);
		last_status = current_switch_status;
		return true;
	}
	return false;
}

void hex_to_string(char *string, const uint8_t *hex, uint8_t len)
{
	uint8_t high,low;
	for(uint8_t i = 0; i < len; i++)
	{
		high = hex[i] >> 4;
		low = hex[i] & 0x0f;
		
		if(high > 9)
			string[i * 2] = high + 'A' - 0x0a;
		else 
			string[i * 2] = high + '0';
		
		if(low > 9)
			string[i * 2 + 1] = low + 'A' - 0x0a;
		else 
			string[i * 2 + 1] = low + '0';
	}
}

void printf_hex_buff(uint8_t *buff,uint8_t len)
{
	static char print_buff[128];
	
	len &= 0x3f;
	
	hex_to_string(print_buff,buff,len);
	print_buff[len * 2] = '\0';
	printf_log(0,"%s\n",print_buff);
}

#define FRAME_HEADR 	0x55aa
#define FRAME_HEAD_LEN	6
#define VERSION			0

typedef struct{
	uint16_t headr;
	uint8_t version;
	uint8_t cmd;
	uint16_t length;/*大端序*/
	uint8_t *data;
	uint8_t checksums;
	
}Ble_Frame;

typedef struct
{
	uint8_t dpid;
	uint8_t type;
	uint16_t len;
	uint8_t value[4];
}Dp_Data;

uint8_t data_check_sum(uint8_t *data, uint8_t len)
{
	uint8_t sum = 0;
	for(uint8_t i = 0; i < len; i++)
	{
		sum += data[i]; 
	}
	return sum;
}

bool check_frame_data(Ble_Frame *frame, uint8_t *buff, uint16_t len)
{
	uint8_t headr[2] = {0x55, 0xaa};
	uint8_t *tmp_p = buff;
	uint16_t index = 0;
	
	for(index = 0; index < len - sizeof(headr); index++,tmp_p++)
	{
		if(memcmp(tmp_p, headr, sizeof(headr)) == 0)
			break;
	}
	
	if(len - index < FRAME_HEAD_LEN)
	{
		printf_log(0,"frame headr error \n");
		return false;
	}
	memcpy(frame, tmp_p, FRAME_HEAD_LEN);
	frame->data = tmp_p + FRAME_HEAD_LEN;
	frame->length = BigLittleSwap16(frame->length);
	if(frame->length > len - 6)
	{
		printf_log(0,"frame len error \n");
		return false;		
	}
		
	frame->checksums = tmp_p[FRAME_HEAD_LEN + frame->length];
	if(frame->checksums != data_check_sum(tmp_p,frame->length + FRAME_HEAD_LEN))
	{
		printf_log(0,"frame check error \n");
		return false;				
	}
	return true;
}	

void send_splicing_data(uint8_t cmd, void *data, uint16_t len)
{
	uint8_t temp_buff[40];
	
	printf_log(0,"BLE TX cmd:0x%02x len:%d\n", cmd, len);
	if(len > sizeof(temp_buff) - FRAME_HEAD_LEN)
	{
		printf_log(0,"send len error \n");
		return ;
	}
	temp_buff[0] = 0x55;
	temp_buff[1] = 0xaa;
	temp_buff[2] = VERSION;
	temp_buff[3] = cmd;
	temp_buff[4] = len >> 8;	
	temp_buff[5] = len & 0xff;
	memcpy(temp_buff + FRAME_HEAD_LEN, data, len);
	temp_buff[FRAME_HEAD_LEN + len] = data_check_sum(temp_buff,FRAME_HEAD_LEN + len);
	
	send_ble_data(temp_buff, len + FRAME_HEAD_LEN + 1);
	//printf_hex_buff(temp_buff, len + FRAME_HEAD_LEN + 1);
}

void heartbeat_response(Ble_Frame *frame)
{
	static bool is_first = true;
	uint8_t status = 1;
	if(is_first)
	{
		status = 0;
		is_first = false;
	}
	send_splicing_data(frame->cmd, &status, 1);
}

void module_status_response(Ble_Frame *frame)
{
	printf_log(0,"module status: %d\n", frame->data[0]);
	send_splicing_data(frame->cmd, NULL, 0);
}

void device_status_response(Ble_Frame *frame)
{
	printf_log(0,"upload %d\n",frame->data[0]);
}

void device_info_response(Ble_Frame *frame)
{
	uint8_t pid_version[] = "vpyk7hkv1.0.0";
	
	send_splicing_data(frame->cmd, pid_version, strlen((char *)pid_version));
}

void upload_status_data()
{
	Dp_Data dp_data[3];
	
	dp_data[0].dpid = 1;	/*温度*/
	dp_data[0].type = 0x02;/*int*/
	dp_data[0].len = BigLittleSwap16(0x0004); /*4byte 大端*/
	current_temperature = BigLittleSwap32(current_temperature);
	memcpy(dp_data[0].value, &current_temperature, sizeof(current_temperature));
	
	dp_data[1].dpid = 2;	/*湿度*/
	dp_data[1].type = 0x02; /*int*/
	dp_data[1].len = BigLittleSwap16(0x0004); /*4byte 大端*/
	current_humidity = BigLittleSwap32(current_humidity);
	memcpy(dp_data[1].value, &current_humidity, sizeof(current_humidity));

	dp_data[2].dpid = 4;	/*电量*/
	dp_data[2].type = 0x02; /*int*/
	dp_data[2].len = BigLittleSwap16(0x0004); /*4byte 大端*/
	battery_percentage = BigLittleSwap32(battery_percentage);
	memcpy(dp_data[2].value, &battery_percentage, sizeof(battery_percentage));
	
	send_splicing_data(7, &dp_data, sizeof(dp_data));
}

void upload_switch_status_data()
{
	Dp_Data dp_data;
	dp_data.dpid = 101;	/*门状态*/
	dp_data.type = 0x01; /*bool*/
	dp_data.len = BigLittleSwap16(0x0001); /*1byte*/
	dp_data.value[0] = current_switch_status; 
	
	send_splicing_data(7, &dp_data, sizeof(dp_data) - 3);	
}

void handle_ble_data()
{
	Ble_Frame frame;
	static uint8_t recv_buff[64],len = 0;
	len = get_recv_buff_data(recv_buff,sizeof(recv_buff) - 1);

//	printf_log(0, "recv len:%d\n",len);
//	printf_hex_buff(recv_buff,len);
	
	if(!check_frame_data(&frame,recv_buff,len))
		return ;
	
	printf_log(0,"BLE RX cmd:0x%02x len:%d\n", frame.cmd, frame.length);
	switch(frame.cmd)
	{
		case 0:	/*设备心跳*/
			heartbeat_response(&frame);
			break;
		
		case 1: /*设备信息*/
			device_info_response(&frame);
			break;
		
		case 2: 
			break;
		
		case 3: /*模块状态*/
			module_status_response(&frame);
			break;

		case 6: /*命令下发*/
			break;

		case 7: /*状态上报响应*/
			device_status_response(&frame);
			break;

		case 8: /*状态查询*/
			
			break;
		
		default:
			send_splicing_data(frame.cmd, NULL, 0);
			break;
	}
}

int32_t main(void)
{
	printf_log(0, "Clk_GetHClkFreq:%llu\n",Clk_GetHClkFreq());
	printf_log(0, "Clk_GetPClkFreq:%llu\n",Clk_GetPClkFreq());
	
	Mesh_BLE_Init();
	ADC_Gpio_Init();
	sht30_init();
	delay1ms(5);
	sht30_init();
	delay1ms(5);
	
	Timer0_Config(Clk_GetPClkFreq()/10/256);
	while (1)
    {
		if(timer_collection_data_flag)
		{
			timer_collection_data_flag = false;
			printf_sht30_data();
			printf_bat_voltage();
		}
		
		if(timer_upload_data_flag)
		{
			upload_status_data();
			timer_upload_data_flag = false;
		}
		
		if(printf_switch_status())
		{
			upload_switch_status_data();
		}
		
		if(get_recv_ble_data_finish())
		{
			handle_ble_data();
		}
		
    }
}
