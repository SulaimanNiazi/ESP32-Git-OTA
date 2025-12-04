#include "freertos/FreeRTOS.h" // for pdMS_TO_TICKS
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define LED 14
#define UART_NUM UART_NUM_0
#define TX  1
#define RX  3
#define MAX_SIZE 1024

void app_main(void)
{
    const gpio_config_t led_cfg = {
        .pin_bit_mask   = 1ULL << LED,
        .mode           = GPIO_MODE_OUTPUT
    };
    gpio_config(&led_cfg);

    const uart_config_t uart_cfg = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_cfg);
    uart_set_pin(UART_NUM, TX, RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, MAX_SIZE, 0, 0, NULL, 0);

    char buffer[MAX_SIZE];
    size_t size = 0;

    while(1){
        size = sprintf(buffer, "LED ON\r\n");
        uart_write_bytes(UART_NUM, buffer, size);
        gpio_set_level(LED, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        size = sprintf(buffer, "LED OFF\r\n");
        uart_write_bytes(UART_NUM, buffer, size);
        gpio_set_level(LED, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
