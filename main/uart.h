#ifndef UART_H
#define UART_H

#include <stdbool.h>

#define MAX_SIZE 1024

bool init_uart(const unsigned int uart_num, const unsigned int baudrate, const unsigned int tx_pin, const unsigned int rx_pin);
bool uart_write(const unsigned int uart_num, const char* buffer);

#endif
