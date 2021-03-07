#include "switch_adc.h"
#include "bsp.h"
bool get_switch_status(void)
{
	return Gpio_GetIO(3,4);
}

void ADC_Gpio_Init(void)
{
    stc_adc_cfg_t       stcAdcCfg;
    stc_adc_cont_cfg_t  stcAdcContCfg;
    
    DDL_ZERO_STRUCT(stcAdcCfg);
    DDL_ZERO_STRUCT(stcAdcContCfg);
    
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);    //GPIO 外设时钟使能
	Clk_SetPeripheralGate(ClkPeripheralAdcBgr, TRUE);  //ADCBGR 外设时钟使能

    Gpio_SetAnalog(2, 4, TRUE);
    
    Adc_Enable();
    M0P_BGR->CR_f.BGR_EN = 0x1u;  //BGR必须使能
    M0P_BGR->CR_f.TS_EN = 0x0u;   //内置温度传感器，视使用需求
    delay100us(1);
    
    stcAdcCfg.enAdcOpMode = AdcContMode;                //连续采样模式
    stcAdcCfg.enAdcClkSel = AdcClkSysTDiv1;             //PCLK
    stcAdcCfg.enAdcSampTimeSel = AdcSampTime12Clk;      //12个采样时钟
    // stcAdcCfg.enAdcRefVolSel = RefVolSelInBgr2p5;    //参考电压:内部2.5V(avdd>3V,SPS<=200kHz)
    stcAdcCfg.enAdcRefVolSel = RefVolSelAVDD;           //参考电压:AVDD
    stcAdcCfg.bAdcInBufEn = FALSE;                      //电压跟随器如果使能，SPS采样速率 <=200K
    stcAdcCfg.enAdcTrig0Sel = AdcTrigDisable;           //ADC转换自动触发设置
    stcAdcCfg.enAdcTrig1Sel = AdcTrigDisable;
    Adc_Init(&stcAdcCfg);    
    
    stcAdcContCfg.enAdcContModeCh = AdcExInputCH0;      //通道1 P26
    stcAdcContCfg.u8AdcSampCnt = 0x09u;                 //P24 连续累加次数(次数 = 0x09+1)
    stcAdcContCfg.bAdcResultAccEn = TRUE;               //累加使能
    Adc_ConfigContMode(&stcAdcCfg, &stcAdcContCfg);
	
	Gpio_InitIO(3,4,GpioDirIn);
}

uint16_t get_bat_adc_value(void)
{
	uint32_t u32AdcResultAcc = 0;
	Adc_Start();                               //ADC开始转换

	while(FALSE == M0P_ADC->IFR_f.CONT_INTF);  //等待转换完成
	M0P_ADC->ICLR_f.CONT_INTC = 0;             //清除转换完成标志位
	
	Adc_GetAccResult(&u32AdcResultAcc);
	Adc_ClrAccResult();
	return u32AdcResultAcc/10;
}

