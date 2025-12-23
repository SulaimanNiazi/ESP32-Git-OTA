#include "nvs.h"
#include "gpio.h"
#include "log.h"

#include <nvs_flash.h>

static void boot_handler(void *arg){
    while(1){
        if(!gpio_get_level(BOOT_PIN)) nvs_flash_erase();
        delay_ms(10);
    }
}

void init_nvs(){
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        error = nvs_flash_init();
    } else log_error(NVS_LOG_TAG, error, "Failed to initialize NVS");

    if(set_pin(BOOT_PIN, GPIO_MODE_INPUT, true, false)) xTaskCreate(boot_handler, "boot_handler", 2048, NULL, 10, NULL);
    else log_error(NVS_LOG_TAG, ESP_FAIL, "Failed to create boot pin handler.");
}

bool nvs_write(const char *key, const char *value){
    bool success = false;
    nvs_handle_t handle;
    if(!log_error(NVS_LOG_TAG, nvs_open(NAMESPACE, NVS_READWRITE, &handle), "Failed to open NVS in rread-write mode.")){
        if(!log_error(NVS_LOG_TAG, nvs_set_str(handle, key, value), "Failed to set string for given key."))
        if(!log_error(NVS_LOG_TAG, nvs_commit(handle), "NVS commit failed")) success = true;
        nvs_close(handle);
    }
    return success;
}

char *nvs_read(const char *key){
    char *buffer = "";
    nvs_handle_t handle;
    if(!log_error(NVS_LOG_TAG, nvs_open(NAMESPACE, NVS_READONLY, &handle), "Failed to open NVS in read mode")){
        size_t size = 0;
        if(!log_error(NVS_LOG_TAG, nvs_get_str(handle, key, NULL, &size), "Failed to get string size")){
            if(size){
                buffer = malloc(size);
                if(log_error(NVS_LOG_TAG, nvs_get_str(handle, key, buffer, &size), "Failed to get string value for given key")){
                    free(buffer);
                    buffer = NULL;
                }
            }
        }
        nvs_close(handle);
    }
    return buffer;
}
