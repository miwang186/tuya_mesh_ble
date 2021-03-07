#ifndef __MESH_BLE_H__
#define __MESH_BLE_H__
#include "base_types.h"

void Mesh_BLE_Init(void);

void send_ble_data(uint8_t *buff, uint8_t len);
uint8_t get_recv_buff_data(uint8_t *buff, uint8_t len);
bool get_recv_ble_data_finish(void);

#endif 
