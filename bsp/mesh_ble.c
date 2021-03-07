#include "mesh_ble.h"
#include "bsp.h"
#include "common.h"
#define RX_BUFF_MAX 64
static uint8_t u8RxData[RX_BUFF_MAX]={0};
static uint16_t uart_recv_time_arr = 0;
static uint8_t rx_data_len = 0;
static bool recv_data_finish = false;

bool get_recv_ble_data_finish(void)
{
	return recv_data_finish;
}

uint8_t get_recv_buff_data(uint8_t *buff, uint8_t len)
{
	uint8_t recv_len = rx_data_len;
	if(rx_data_len > len )
		recv_len = len;
	
	memcpy(buff,u8RxData, recv_len);
	
	rx_data_len = 0;
	recv_data_finish = false;
	return recv_len;
}

void RxIntCallback(void)
{
	u8RxData[rx_data_len] = M0P_LPUART->SBUF;

	if(rx_data_len < RX_BUFF_MAX)
	{
		Bt_Cnt16Set(TIM1, uart_recv_time_arr);
		if(rx_data_len == 0)
			Bt_Run(TIM1);
		rx_data_len++;
	}
	else 
	{
		recv_data_finish = true;
		Bt_Stop(TIM1);
	}
}

void ErrIntCallback(void)
{
  
}
void send_ble_data(uint8_t *buff, uint8_t len)
{
	while(len--)
		LPUart_SendData(*buff++);
}
/*******************************************************************************
 * BT1中断服务函数
 ******************************************************************************/
static void Bt1IntCallback(void)
{
    if (TRUE == Bt_GetIntFlag(TIM1))
    {
		recv_data_finish = true;
		Bt_Stop(TIM1);
		Bt_ClearIntFlag(TIM1);
    }
}

void Mesh_BLE_Power_en()
{
	
}

static void UartTimer1Cfg(uint16_t u16Period)
{
    stc_bt_config_t   stcConfig;

	Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);
    stcConfig.pfnTim1Cb = Bt1IntCallback;
        
    stcConfig.enPRS   = BtPCLKDiv2;
    stcConfig.enCT    = BtTimer;
    stcConfig.enMD    = BtMode2;
    //Bt初始化
	Bt_Init(TIM1, &stcConfig);
    
    //TIM1中断使能
    Bt_ClearIntFlag(TIM1);
    Bt_EnableIrq(TIM1);
    EnableNvic(TIM1_IRQn, 3, TRUE);
	
	uart_recv_time_arr = 0x10000 - u16Period;
	
    //设置重载值和计数值，启动计数
    Bt_ARRSet(TIM1, uart_recv_time_arr);
    Bt_Cnt16Set(TIM1, uart_recv_time_arr);
//    Bt_Run(TIM1);
}

void Mesh_BLE_Init()
{
	uint16_t u16timer;
    uint32_t u32sclk;
    stc_lpuart_config_t  stcConfig;
    stc_lpuart_irq_cb_t stcLPUartIrqCb;
    stc_lpuart_multimode_t stcMulti;
    stc_lpuart_sclk_sel_t  stcLpuart_clk;
    stc_lpuart_mode_t       stcRunMode;
    stc_lpuart_baud_config_t  stcBaud;
    stc_bt_config_t stcBtConfig;
    
    DDL_ZERO_STRUCT(stcConfig);
    DDL_ZERO_STRUCT(stcLPUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBtConfig);
    
    Clk_SetPeripheralGate(ClkPeripheralLpUart,TRUE);//使能LPUART时钟
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);

	Gpio_InitIO(0,3,GpioDirOut);
	Gpio_SetIO(0,3,FALSE);
    
    //通道端口配置
    Gpio_InitIOExt(2,5,GpioDirOut,TRUE,FALSE,FALSE,FALSE);
    Gpio_InitIOExt(2,6,GpioDirOut,TRUE,FALSE,FALSE,FALSE);

    Gpio_SetFunc_UART2RX_P25();
    Gpio_SetFunc_UART2TX_P26();    
   
    stcLpuart_clk.enSclk_sel = LPUart_Pclk;//LPUart_Rcl;
    
    stcLpuart_clk.enSclk_Prs = LPUartDiv1;
    stcConfig.pstcLpuart_clk = &stcLpuart_clk;

    stcRunMode.enLpMode = LPUartNoLPMode;//正常工作模式或低功耗工作模式配置
    stcRunMode.enMode   = LPUartMode1;
    stcConfig.pstcRunMode = &stcRunMode;

    stcLPUartIrqCb.pfnRxIrqCb = RxIntCallback;
    stcLPUartIrqCb.pfnTxIrqCb = NULL;
    stcLPUartIrqCb.pfnRxErrIrqCb = ErrIntCallback;
    stcConfig.pstcIrqCb = &stcLPUartIrqCb;
    stcConfig.bTouchNvic = TRUE;

    stcMulti.enMulti_mode = LPUartNormal;//只有模式2/3才有多主机模式

    stcConfig.pstcMultiMode = &stcMulti;
   
    LPUart_EnableIrq(LPUartRxIrq);

    LPUart_Init(&stcConfig);

    if(LPUart_Pclk == stcLpuart_clk.enSclk_sel)
        u32sclk = Clk_GetPClkFreq();
    else if(LPUart_Rcl == stcLpuart_clk.enSclk_sel)
        u32sclk = 38400;//此处建议用户使用内部38.4K时钟，如果用户使用32.768K时钟的，此处更新成32768
    else
        u32sclk = 32768;
      
    stcBaud.u32Baud = 115200;
    stcBaud.bDbaud = 0;
    stcBaud.u8LpMode = LPUartNoLPMode;
    stcBaud.u8Mode = LPUartMode1;
    u16timer = LPUart_SetBaudRate(u32sclk,stcLpuart_clk.enSclk_Prs,&stcBaud);
    stcBtConfig.enMD = BtMode2;
    stcBtConfig.enCT = BtTimer;
    stcBtConfig.enTog = BtTogEnable;
    Bt_Init(TIM2, &stcBtConfig);//调用basetimer2设置函数产生波特率
    Bt_ARRSet(TIM2,u16timer);
    Bt_Cnt16Set(TIM2,u16timer);
    Bt_Run(TIM2);

    LPUart_EnableFunc(LPUartRx);
	UartTimer1Cfg(Clk_GetPClkFreq()/2/200);//5ms	
}
