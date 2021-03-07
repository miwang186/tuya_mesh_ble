#ifndef __SHT30_H__
#define __SHT30_H__
#include "base_types.h"
bool read_sht30_temp(float *temp, float *hium);
void sht30_init(void);
#endif 
