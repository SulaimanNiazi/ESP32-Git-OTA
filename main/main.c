#include "delay.h"
#include "gpio.h"
#include "uart.h"
#include "nvs.h"

#define led_pin         14
#define uart_num        0
#define tx_pin          1
#define rx_pin          3

void app_main(){
    set_pin(led_pin, GPIO_MODE_OUTPUT, false, false);
    init_uart(uart_num, 115200, tx_pin, rx_pin);
    init_nvs();

    char *wifi_ssid = nvs_read("wifi_ssid");
    char *wifi_pin = nvs_read("wifi_pin");
    if(wifi_ssid[0]){
        uart_write(uart_num, "SSID: "); uart_write(uart_num, wifi_ssid);
        uart_write(uart_num, "\r\nPin: "); uart_write(uart_num, wifi_pin);
    } else {
        uart_write(uart_num, "WiFi SSID not found.\r\n");
        char* ssid = uart_read(uart_num, "Enter WiFi SSID: ");
        nvs_write("wifi_ssid", ssid);
        char *pin = uart_read(uart_num, "Enter WiFi Pin: ");
        nvs_write("wifi_pin", pin);
    }

    while(1){
        gpio_set_level(led_pin, 1);
        delay_ms(500);
        gpio_set_level(led_pin, 0);
        delay_ms(500);
    }
}
