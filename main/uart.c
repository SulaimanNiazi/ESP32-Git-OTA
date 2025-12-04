#include "uart.h"
#include "driver/uart.h"

bool init_uart(const unsigned int uart_num, const unsigned int baudrate, const unsigned int tx_pin, const unsigned int rx_pin){
    const uart_config_t uart_cfg = {
        .baud_rate  = baudrate,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE
    };

    esp_err_t errors = 0;
    errors += uart_param_config(uart_num, &uart_cfg);
    errors += uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    errors += uart_driver_install(uart_num, MAX_SIZE, 0, 0, NULL, 0);
    return errors == 0;
}

bool uart_write(const unsigned int uart_num, const char* buffer){
    size_t size = 0;
    for(; buffer[size] != '\0'; size++);
    return uart_write_bytes(uart_num, buffer, size) == 0;
}
