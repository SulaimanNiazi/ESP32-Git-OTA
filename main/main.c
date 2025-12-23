#include "gpio.h"
#include "uart.h"
#include "nvs.h"
#include "wifi.h"
#include "ota.h"
#include "log.h"

#define uart_num        0
#define tx_pin          1
#define rx_pin          3
#define led_pin         2

char *nvs_uart_read(const char *key, const char *name){
    char *prev = nvs_read(key);
    if(prev[0]) return prev;
    
    while(1){
        uart_printf(uart_num, "\r\n%s not found, Enter %s: ", name, name);
        char *value = uart_read(uart_num);
        if(value[0]){
            bool success = nvs_write(key, value);
            if(success){
                uart_printf(uart_num, "Saved successfully\r\n");
                size_t size = 1;
                for(; value[size]; size++);
                char *output = malloc(size);
                sprintf(output, "%s", value);
                return output;
            }else{
                uart_printf(uart_num, "Save failed\r\n");
                return "";
            }
        }else uart_printf(uart_num, "Invalid input\r\n");
    }
}

void connect_wifi(){
    while(!wifi_connected()){
        uart_printf(uart_num, ".");
        delay_ms(500);
    }

    const char *ip = wifi_get_ip(), *gw = wifi_get_gateway(), *mask = wifi_get_netmask();
    uart_printf(uart_num, "\r\nWiFi Connected.\r\nIP: \t %s\r\nGateway: %s\r\nNetmask: %s\r\n", ip, gw, mask);
}

void app_main(){
    init_log(3, GPIO_LOG_TAG, UART_LOG_TAG, OTA_LOG_TAG);
    init_uart(uart_num, 115200, tx_pin, rx_pin);
    init_nvs();

    const char *wifi_ssid = nvs_uart_read("wifi_ssid", "WiFi SSID"), *wifi_pin = nvs_uart_read("wifi_pin", "WiFi Pin");
    init_wifi(wifi_ssid, wifi_pin);

    uart_printf(uart_num, "\r\n\r\nConnecting to %s ...", wifi_ssid);
    connect_wifi();

    check_ota();
    const char *current = ota_get_version(false), *latest = ota_get_version(true);
    uart_printf(uart_num, "\r\nCurrent version: %s\r\nLatest version:  %s\r\n", current, latest);

    if(ota_up_to_date()){
        uart_printf(uart_num, "Up to date\r\n");
    }
    else if(wifi_connected()){
        uart_printf(uart_num, "Not up to date\r\nUpdating...\r\n");
        ota_update();
    }

    set_pin(led_pin, GPIO_MODE_OUTPUT, false, false);
    while(1){
        gpio_set_level(led_pin, 1);
        delay_ms(500);
        gpio_set_level(led_pin, 0);
        delay_ms(500);
    }
}
