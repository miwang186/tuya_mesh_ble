#include "sht30.h"
#include "hardware_i2c.h"
#include "common.h"

#define SHT30_WRITE_ADDR 0x88 //地址

#define CMD_FETCH_DATA_H 0x22 //循环采样，参考sht30 datasheet

#define CMD_FETCH_DATA_L 0x36

bool read_sht30_temp(float *temp, float *hium)
{
	en_result_t  result;
	uint8_t sht30_buf[6] = {0};
	result = I2C_MasterReadData(SHT30_WRITE_ADDR,sht30_buf,6);
	if(result != Ok)
	{
		printf_log(0,"read sht30 data fail\n");
		return false;
	}	
	*temp = ((((sht30_buf[0]*256) + sht30_buf[1]) * 175)/65535.0 - 45);
	*hium = (((sht30_buf[3]*256) + (sht30_buf[4]) ) * 100/65535.0);
	return true;
}

void sht30_init(void)
{
	uint8_t write_buff[2] ={CMD_FETCH_DATA_H, CMD_FETCH_DATA_L};
	I2C_Master_Init();
	I2C_MasterWriteData(SHT30_WRITE_ADDR,write_buff,2);
}
