#ifndef __SWITCH_ADC_H__
#define __SWITCH_ADC_H__
#include "base_types.h"

uint16_t get_bat_adc_value(void);
void ADC_Gpio_Init(void);
bool get_switch_status(void);

#endif 
