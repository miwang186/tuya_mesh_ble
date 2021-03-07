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
    
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);    //GPIO ����ʱ��ʹ��
	Clk_SetPeripheralGate(ClkPeripheralAdcBgr, TRUE);  //ADCBGR ����ʱ��ʹ��

    Gpio_SetAnalog(2, 4, TRUE);
    
    Adc_Enable();
    M0P_BGR->CR_f.BGR_EN = 0x1u;  //BGR����ʹ��
    M0P_BGR->CR_f.TS_EN = 0x0u;   //�����¶ȴ���������ʹ������
    delay100us(1);
    
    stcAdcCfg.enAdcOpMode = AdcContMode;                //��������ģʽ
    stcAdcCfg.enAdcClkSel = AdcClkSysTDiv1;             //PCLK
    stcAdcCfg.enAdcSampTimeSel = AdcSampTime12Clk;      //12������ʱ��
    // stcAdcCfg.enAdcRefVolSel = RefVolSelInBgr2p5;    //�ο���ѹ:�ڲ�2.5V(avdd>3V,SPS<=200kHz)
    stcAdcCfg.enAdcRefVolSel = RefVolSelAVDD;           //�ο���ѹ:AVDD
    stcAdcCfg.bAdcInBufEn = FALSE;                      //��ѹ���������ʹ�ܣ�SPS�������� <=200K
    stcAdcCfg.enAdcTrig0Sel = AdcTrigDisable;           //ADCת���Զ���������
    stcAdcCfg.enAdcTrig1Sel = AdcTrigDisable;
    Adc_Init(&stcAdcCfg);    
    
    stcAdcContCfg.enAdcContModeCh = AdcExInputCH0;      //ͨ��1 P26
    stcAdcContCfg.u8AdcSampCnt = 0x09u;                 //P24 �����ۼӴ���(���� = 0x09+1)
    stcAdcContCfg.bAdcResultAccEn = TRUE;               //�ۼ�ʹ��
    Adc_ConfigContMode(&stcAdcCfg, &stcAdcContCfg);
	
	Gpio_InitIO(3,4,GpioDirIn);
}

uint16_t get_bat_adc_value(void)
{
	uint32_t u32AdcResultAcc = 0;
	Adc_Start();                               //ADC��ʼת��

	while(FALSE == M0P_ADC->IFR_f.CONT_INTF);  //�ȴ�ת�����
	M0P_ADC->ICLR_f.CONT_INTC = 0;             //���ת����ɱ�־λ
	
	Adc_GetAccResult(&u32AdcResultAcc);
	Adc_ClrAccResult();
	return u32AdcResultAcc/10;
}

