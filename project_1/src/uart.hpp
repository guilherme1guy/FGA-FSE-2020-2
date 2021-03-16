#ifndef UART_H_
#define UART_H_

#include "modbus.hpp"

ModbusMessage* uart_communication(unsigned char *data, int data_length);

#endif