#include "delay.h"
#include "gpio.h"
#include "uart.h"
#include "nvs.h"

#define led_pin         14
#define uart_num        0
#define tx_pin          1
#define rx_pin          3

char* nvs_uart_read(const char* key, const char* name){
    char* prev = nvs_read(key);
    if(prev[0]) return prev;
    
    char prompt[NVS_MAX_LENGTH];
    while(1){
        sprintf(prompt, "\r\n%s not found, Enter %s: ", name, name);
        char* value = uart_read(uart_num, prompt);
        if(value[0]){
            bool success = nvs_write(key, value);
            if(success){
                uart_write(uart_num, "Saved successfully\r\n");
                size_t size = 1;
                for(; value[size]; size++);
                char* output = malloc(size);
                sprintf(output, "%s", value);
                return output;
            }else{
                uart_write(uart_num, "Save failed\r\n");
                return "";
            }
        }else uart_write(uart_num, "Invalid input\r\n");
    }
}

void app_main(){
    set_pin(led_pin, GPIO_MODE_OUTPUT, false, false);
    init_uart(uart_num, 115200, tx_pin, rx_pin);
    init_nvs();

    char* wifi_ssid = nvs_uart_read("wifi_ssid", "WiFi SSID"),* wifi_pin = nvs_uart_read("wifi_pin", "WiFi Pin");
    uart_write(uart_num, wifi_ssid);
    uart_write(uart_num, " ");
    uart_write(uart_num, wifi_pin);

    while(1){
        gpio_set_level(led_pin, 1);
        delay_ms(500);
        gpio_set_level(led_pin, 0);
        delay_ms(500);
    }
}
