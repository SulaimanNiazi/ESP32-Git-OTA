#pragma once

#include "delay.h"

#define UART_MAX_LENGTH 1024
#define UART_LOG_TAG    "UART"

void init_uart(const size_t uart_num, const size_t baudrate, const size_t tx_pin, const size_t rx_pin);
void uart_printf(const size_t uart_num, const char* format, ...);
char *uart_read(const size_t uart_num);
