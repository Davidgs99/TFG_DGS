#include "modbus_params.h"  // for modbus parameters structures
#include "mbcontroller.h"
#include "esp_modbus_common.h"
#include "esp_modbus_master.h"

#ifndef MODBUS_MASTER_h
#define MODBUS_MASTER_h

#define MB_UART_PORT_NUM 2 
#define MB_UART_BAUD_RATE 115200 
#define MB_UART_RXD 22               //DEFAULT UART PORT NUMBER
#define MB_UART_TXD 23               //DEFAULT UART PORT NUMBER
#define MB_UART_RTS 18               //DEFAULT UART PORT NUMBER

esp_err_t master_init(void); // Modbus master initialization
void* master_get_param_data(const mb_parameter_descriptor_t* param_descriptor); // The function to get pointer to parameter storage (instance) according to parameter description table
void master_operation_func(void *arg); // User operation function to read slave values and check alarm
void modbus_receiver_check_function(void); // Initialization of device peripheral and objects

#endif
