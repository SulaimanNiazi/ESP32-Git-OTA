#include "delay.h"
#include "gpio.h"
#include "uart.h"

#define led_pin 14
#define uart_num 0
#define tx_pin  1
#define rx_pin  3

void app_main(){
    set_pin(led_pin, GPIO_MODE_OUTPUT);
    init_uart(uart_num, 115200, tx_pin, rx_pin);

    while(1){
        // uart_write(uart_num, "LED ON\r\n"); 
        // gpio_set_level(led_pin, 1);
        // delay_ms(500);
        
        // uart_write(uart_num, "LED OFF\r\n");
        // gpio_set_level(led_pin, 0);

        char* entry = uart_read(uart_num, "Enter: ");
        uart_write(uart_num, entry);
        uart_write(uart_num, "\r\n");
        delay_ms(5);
    }
}
