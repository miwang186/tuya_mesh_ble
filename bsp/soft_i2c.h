#ifndef		__SOFT_I2C_H__
#define 	__SOFT_I2C_H__
#include "base_types.h"
void I2C_GPIOPortCfg(void);
bool I2C0_ReadNumByte(uint8_t slave_addr, uint8_t *pbuff, uint8_t mem_addr, uint8_t length);
bool I2C0_WriteNumByte(uint8_t slave_addr, uint8_t *pbuff, uint8_t mem_addr, uint8_t length);

#endif
