#include "bsp.h"
#include "hardware_i2c.h"

/**
 ******************************************************************************
 ** \brief  主机接收函数
 **
 ** \param pu8Data读数据存放缓存，u32Len读数据长度
 **
 ** \retval 读数据是否成功
 **
 ******************************************************************************/

en_result_t I2C_MasterReadData(uint8_t slaveaddr,uint8_t *pu8Data,uint32_t u32Len)
{
    en_result_t enRet = Error;
    uint8_t u8i=0,u8State;
    uint16_t time_out = 0xfff,try_count = 10;
    I2C_SetFunc(I2cStart_En);
    
	while(1)
	{
		if(try_count == 0)
			return ErrorTimeout;
		
		while(0 == I2C_GetIrq() && time_out != 0)
        {
			time_out--;
			if(time_out == 0)
				return ErrorTimeout;
		}
		u8State = I2C_GetState();
		switch(u8State)
		{
			case 0x08:
			case 0x10:
				I2C_ClearFunc(I2cStart_En);
				I2C_WriteByte(slaveaddr|0x01);//从机地址发送OK
				try_count--;
				break;
			case 0x40:
				if(u32Len>1)
				{
					I2C_SetFunc(I2cAck_En);
				}
				break;
			case 0x50:
				pu8Data[u8i++] = I2C_ReadByte();
				if(u8i==u32Len-1)
				{
					I2C_ClearFunc(I2cAck_En);
				}
				break;
			case 0x58:
				pu8Data[u8i++] = I2C_ReadByte();
				I2C_SetFunc(I2cStop_En);
				break;	
			case 0x38:
				I2C_SetFunc(I2cStart_En);
				break;
			case 0x48:
				I2C_SetFunc(I2cStop_En);
				I2C_SetFunc(I2cStart_En);
				break;
			default:
				I2C_SetFunc(I2cStart_En);//其他错误状态，重新发送起始条件
				break;
		}
		I2C_ClearIrq();
		if(u8i==u32Len)
		{
			break;
		}
	}
	enRet = Ok;
	return enRet;
}
/**
 ******************************************************************************
 ** \brief  主机发送函数
 **
 ** \param pu8Data写数据，u32Len写数据长度
 **
 ** \retval 写数据是否成功
 **
 ******************************************************************************/
en_result_t I2C_MasterWriteData(uint8_t slaveaddr, uint8_t *pu8Data,uint32_t u32Len)
{
    en_result_t enRet = Error;
    uint8_t u8i=0,u8State;
	uint16_t try_count = 10;
    I2C_SetFunc(I2cStart_En);
	while(1)
	{
		if(try_count == 0)
			return ErrorTimeout;
		
		while(0 == I2C_GetIrq());
		
		u8State = I2C_GetState();
		switch(u8State)
		{
			case 0x08:
			case 0x10:
				I2C_ClearFunc(I2cStart_En);
				I2C_WriteByte(slaveaddr);//从设备地址发送
				try_count--;
				break;
			case 0x18:
			case 0x28:	
				I2C_WriteByte(pu8Data[u8i++]);
				break;
			case 0x20:
			case 0x38:
				I2C_SetFunc(I2cStart_En);
				break;
			case 0x30:
				I2C_SetFunc(I2cStop_En);
				I2C_SetFunc(I2cStart_En);
				break;
			default:
				break;
		}			
		if(u8i>u32Len)
		{
			I2C_SetFunc(I2cStop_En);//此顺序不能调换，出停止条件
			I2C_ClearIrq();
			break;
		}
		I2C_ClearIrq();	
		delay100us(1);
	}
    enRet = Ok;
    return enRet;
}


void I2C_Master_Init(void)
{
	stc_i2c_config_t stcI2cCfg;

    DDL_ZERO_STRUCT(stcI2cCfg);

    Gpio_InitIOExt(1,4,GpioDirOut,FALSE,FALSE,TRUE,FALSE);   
    Gpio_InitIOExt(1,5,GpioDirOut,FALSE,FALSE,TRUE,FALSE);
    
    Gpio_SetFunc_I2CDAT_P15(); 
    Gpio_SetFunc_I2C_SCL_P14();

    Clk_SetPeripheralGate(ClkPeripheralI2c,TRUE);
    
    stcI2cCfg.enFunc = I2cBaud_En;
    stcI2cCfg.u8Tm = 0x03;//1M波特率配置
    stcI2cCfg.pfnI2cCb = NULL;
    stcI2cCfg.bTouchNvic = FALSE;
    if(stcI2cCfg.bTouchNvic == TRUE)
	{
		EnableNvic(I2C_IRQn,3,TRUE);
	}
    I2C_DeInit();
    
    I2C_Init(&stcI2cCfg);
    I2C_SetFunc(I2cHlm_En);
    I2C_SetFunc(I2cMode_En);	
}
