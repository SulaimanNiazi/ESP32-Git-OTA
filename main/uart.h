#pragma once

#include "delay.h"

#define UART_MAX_LENGTH 1024

bool init_uart(const unsigned int uart_num, const unsigned int baudrate, const unsigned int tx_pin, const unsigned int rx_pin);
int uart_write(const unsigned int uart_num, const char *buffer);
char *uart_read(const unsigned int uart_num, const char *prompt);
