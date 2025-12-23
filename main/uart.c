#include "uart.h"
#include "log.h"

#include "driver/uart.h"
#include <stdarg.h>

typedef struct{
    char buffer[UART_MAX_LENGTH + 1];
    volatile bool receiving;
    volatile size_t count;
    QueueHandle_t queue;
    TaskHandle_t handler;
} uart_t;

static uart_t uarts[3];

void uart_event_task(void *arg){
    uart_event_t event;
    uint8_t data[128];
    const uart_port_t uart_num = *((uart_port_t*)arg);
    size_t count = 0;

    while(1){
        if(xQueueReceive(uarts[uart_num].queue, (void*)&event, portMAX_DELAY)){
            switch(event.type){
                case UART_DATA:
                    int len = uart_read_bytes(uart_num, data, event.size, portMAX_DELAY);
                    if(len >= 0){
                        for(size_t i = 0; i < len; i++){
                            switch(data[i]){
                                case '\n':
                                case '\r':
                                    uart_write_bytes(uart_num, "\r\n", 2);
                                    uarts[uart_num].receiving = (count = uarts[uart_num].buffer[count] = 0);
                                    break;
                                
                                case '\b':
                                    if(count > 0){
                                        uarts[uart_num].buffer[--count] = 0;
                                        uart_write_bytes(uart_num, "\b \b", 3);
                                    }
                                    break;
                                
                                default:
                                    if(count < UART_MAX_LENGTH){
                                        uarts[uart_num].buffer[count++] = data[i];
                                        uart_write_bytes(uart_num, &data[i], 1);
                                    }
                            }
                        }
                    } else ESP_LOGE(UART_LOG_TAG, "Failed to read bytes.");
                    break;
                
                case UART_FIFO_OVF:
                case UART_BUFFER_FULL:
                    log_error(UART_LOG_TAG, uart_flush_input(uart_num), "Failed to flush input");
                    xQueueReset(uarts[uart_num].queue);
                default: break;
            }
        }
    }
}

void init_uart(const size_t uart_num, const size_t baudrate, const size_t tx_pin, const size_t rx_pin){
    const uart_config_t uart_cfg = {
        .baud_rate  = baudrate,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE
    };

    log_error(UART_LOG_TAG, uart_param_config(uart_num, &uart_cfg), "Failed to configure UART");
    log_error(UART_LOG_TAG, uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE), "Failed to assign UART pins");

    uart_t uart = {.receiving = false};
    uarts[uart_num] = uart;

    log_error(
        UART_LOG_TAG,
        uart_driver_install(uart_num, UART_MAX_LENGTH, 0, 20, &uarts[uart_num].queue, 0),
        "Failed to install UART driver"
    );
    xTaskCreate(uart_event_task, "uart_event_task", 4096, (void*const)&uart_num, 12, &uarts[uart_num].handler);
    vTaskSuspend(uarts[uart_num].handler);
}

void uart_printf(const size_t uart_num, const char* format, ...){
    va_list args;
    va_start(args, format);
    size_t size = vsnprintf(uarts[uart_num].buffer, UART_MAX_LENGTH, format, args);
    va_end(args);
    if(uart_write_bytes(uart_num, uarts[uart_num].buffer, size) <= 0) ESP_LOGE(UART_LOG_TAG, "Failed to write bytes");
}

char *uart_read(const size_t uart_num){
    vTaskResume(uarts[uart_num].handler);
    uarts[uart_num].receiving = true;
    while(uarts[uart_num].receiving){
        // Do anything
        delay_ms(10);
    }
    vTaskSuspend(uarts[uart_num].handler);
    return (char*)uarts[uart_num].buffer;
}
