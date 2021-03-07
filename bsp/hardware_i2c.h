#ifndef __HARDWARE_I2C_H__
#define __HARDWARE_I2C_H__

#include "base_types.h"

void I2C_Master_Init(void);
//�����ͺ���
en_result_t I2C_MasterWriteData(uint8_t slaveaddr,uint8_t *pu8Data,uint32_t u32Len);
//�����պ���
en_result_t I2C_MasterReadData(uint8_t slaveaddr,uint8_t *pu8Data,uint32_t u32Len);

#endif 
