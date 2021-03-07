#include "soft_i2c.h"
#include "bsp.h"
#include "common.h"

#define I2C0_SCL_L()		Gpio_SetIO(1, 4, FALSE)
#define I2C0_SCL_H()		Gpio_SetIO(1, 4, TRUE)
                            
#define I2C0_SDA_L()		Gpio_SetIO(1, 5, FALSE)
#define I2C0_SDA_H()		Gpio_SetIO(1, 5, TRUE)

uint8_t SDA_READ()
{
	uint8_t gpio_value = 0;
	setBit((uint32_t)&M0P_GPIO->P0DIR + 1 * GPIO_GPSZ, 5, GpioDirIn);
	gpio_value = Gpio_GetIO(1,5);
	setBit((uint32_t)&M0P_GPIO->P0DIR + 1 * GPIO_GPSZ, 5, GpioDirOut);
	return gpio_value; 
}

///< IO端口配置
void I2C_GPIOPortCfg(void)
{
	Gpio_InitIO(1,4,GpioDirOut);
	Gpio_InitIO(1,5,GpioDirOut);	
}

static void I2C0_delay(void)
{
	__nop();
}

/*******************************************************************************
* Function Name  : I2C0_Start
* Description    : Master Start Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : Wheather  Start
****************************************************************************** */
static bool I2C0_Start(void)
{
    I2C0_SDA_H();
    I2C0_SCL_H();
    I2C0_delay();

    if(!SDA_READ()) return false;

    I2C0_SDA_L();
    I2C0_delay();

//    if(SDA_READ()) return false;

    I2C0_SDA_L();
    I2C0_delay();
    return true;
}

/*******************************************************************************
* Function Name  : I2C0_Stop
* Description    : Master Stop Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
static void I2C0_Stop(void)
{
    I2C0_SCL_L();
    I2C0_delay();
    I2C0_SDA_L();
    I2C0_delay();
    I2C0_SCL_H();
    I2C0_delay();
    I2C0_SDA_H();
    I2C0_delay();
}

/*******************************************************************************
* Function Name  : I2C0_Ack
* Description    : Master Send Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
static void I2C0_Ack(void)
{
    I2C0_SCL_L();
    I2C0_delay();
    I2C0_SDA_L();
    I2C0_delay();
    I2C0_SCL_H();
    I2C0_delay();
    I2C0_SCL_L();
    I2C0_delay();
}

/*******************************************************************************
* Function Name  : I2C0_NoAck
* Description    : Master Send No Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
static void I2C0_NoAck(void)
{
    I2C0_SCL_L();
    I2C0_delay();
    I2C0_SDA_H();
    I2C0_delay();
    I2C0_SCL_H();
    I2C0_delay();
    I2C0_SCL_L();
    I2C0_delay();
}


/*******************************************************************************
* Function Name  : I2C0_WaitAck
* Description    : Master Reserive Slave Acknowledge Single
* Input          : None
* Output         : None
* Return         : Wheather  Reserive Slave Acknowledge Single
****************************************************************************** */
static bool I2C0_WaitAck(void)
{
//	uint8_t time_out = 5;
    I2C0_SCL_L();
    I2C0_delay();
    I2C0_SDA_H();
    I2C0_delay();
    I2C0_SCL_H();
    I2C0_delay();

//	do{
//		if(!SDA_READ())
//			break;
//		I2C0_delay();
//	}while(--time_out);
//	
//	if(time_out == 0)
//	{
//		SEGGER_RTT_printf(0, "I2C0_WaitAck fail! \n");
//		I2C0_SCL_L();
//		I2C0_delay();
//		return false;
//	}

//	SEGGER_RTT_printf(0, "I2C0_WaitAck succ!\n");
	
    I2C0_SCL_L();
    I2C0_delay();
    return true;
}

/*******************************************************************************
* Function Name  : I2C0_SendByte
* Description    : Master Send a Byte to Slave
* Input          : Will Send Date
* Output         : None
* Return         : None
****************************************************************************** */
static void I2C0_SendByte(uint8_t SendByte) /*数据从高位到低位*/
{
    uint8_t i = 8;

    while(i--)
    {
        I2C0_SCL_L();
        I2C0_delay();

        if(SendByte & 0x80)
            I2C0_SDA_H();
        else
            I2C0_SDA_L();

        SendByte <<= 1;

        I2C0_SCL_H();
        I2C0_delay();
    }

    I2C0_SCL_L();
}

/*******************************************************************************
* Function Name  : I2C0_RadeByte
* Description    : Master Reserive a Byte From Slave
* Input          : None
* Output         : None
* Return         : Date From Slave
****************************************************************************** */
static uint8_t I2C0_ReceiveByte(void) /*MSB*/
{
    uint8_t i = 8;
    uint8_t ReceiveByte = 0;
    I2C0_SDA_H();

    while(i --)
    {
        ReceiveByte <<= 1;
        I2C0_SCL_L();
        I2C0_delay();
        I2C0_SCL_H();
        I2C0_delay();

        if(SDA_READ())
        {
            ReceiveByte |= 0x01;
        }
    }

    I2C0_SCL_L();
    return ReceiveByte;
}

bool CheckWriteBusy(uint8_t slave_addr)
{
    uint16_t timout = 300;

    do
    {
        I2C0_Start();
        I2C0_SendByte(slave_addr);
        delay100us(10);
        timout--;

    }
    while(!I2C0_WaitAck() && timout);

    I2C0_Stop();
    return true;
}

void i2c_send_byte(uint8_t dat)
{
    I2C0_Start();
    I2C0_SendByte(dat);
    I2C0_WaitAck();	
	I2C0_Stop();
}
void lcd_9616_Init()
{
	i2c_send_byte(0x21);
	i2c_send_byte(0x16);
	i2c_send_byte(0x21);
	i2c_send_byte(0x0c);
	i2c_send_byte(0x23);
	i2c_send_byte(0x0b);
	i2c_send_byte(0x20);
	i2c_send_byte(0x05);
	i2c_send_byte(0x21);
	i2c_send_byte(0xd0);
	i2c_send_byte(0x20);
	i2c_send_byte(0x0c);
}
/******************************************************************************
/ 函数功能:多字节写函数
/ 修改日期:none
/ 输入参数:
/   @arg SlaveAddress   从器件地址
/   @arg REG_Address    寄存器地址
/   @arg ptChar         输出缓冲
/   @arg size           读出的数据个数,size必须大于=1
/ 输出参数: 成功失败标记
/ 使用说明:none
******************************************************************************/
bool I2C0_WriteNumByte(uint8_t slave_addr, uint8_t *pbuff, uint8_t mem_addr, uint8_t length)
{
    uint8_t i;

    if(!I2C0_Start())return false;

    I2C0_SendByte(slave_addr);

    if(!I2C0_WaitAck())
    {
        I2C0_Stop();
        return false;
    }
	
	if(mem_addr != 0)
	{	
		I2C0_SendByte(mem_addr);
		I2C0_WaitAck();
	}
	
    for(i = 1; i < length; i++)
    {
        I2C0_SendByte(*pbuff);
        I2C0_WaitAck();
        pbuff++;
    }

    I2C0_SendByte(*pbuff);
    I2C0_WaitAck();
    I2C0_Stop();

//    CheckWriteBusy(slave_addr);

    return true;
}

/******************************************************************************
/ 函数功能:多字节读出函数
/ 修改日期:none
/ 输入参数:
/   @arg SlaveAddress   从器件地址
/   @arg REG_Address    寄存器地址
/   @arg ptChar         输出缓冲
/   @arg length           读出的数据个数,length必须大于=1,小于等于8
/ 输出参数: 成功失败标记
/ 使用说明:none
******************************************************************************/
bool I2C0_ReadNumByte(uint8_t slave_addr, uint8_t *pbuff, uint8_t mem_addr, uint8_t length)
{
    uint8_t i;

    if(!I2C0_Start())return false;

	if(mem_addr != 0)
	{	
		I2C0_SendByte(slave_addr);

		if(!I2C0_WaitAck())
		{
			I2C0_Stop();
			return false;
		}

		I2C0_SendByte(mem_addr);
		I2C0_WaitAck();
		
		I2C0_Start();
	}
    I2C0_SendByte(slave_addr + 1);
    I2C0_WaitAck();

    for(i = 1; i < length; i++)
    {
        *pbuff++ = I2C0_ReceiveByte();
        I2C0_Ack();
    }

    *pbuff++ = I2C0_ReceiveByte();
    I2C0_NoAck();
    I2C0_Stop();

    return true;
}
