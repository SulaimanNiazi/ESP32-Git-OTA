#include "nvs.h"
#include "nvs_flash.h"
#include "gpio.h"

static void boot_handler(void *arg){
    while(1){
        if(!gpio_get_level(BOOT_PIN)) nvs_flash_erase();
        delay_ms(10);
    }
}

void error_handler(const esp_err_t error){
    switch(error){
        case ESP_OK: break;
        case ESP_ERR_NO_MEM: break;
        case ESP_ERR_NVS_NOT_ENOUGH_SPACE: break;
        case ESP_ERR_NVS_PAGE_FULL: break;
        case ESP_ERR_NVS_NO_FREE_PAGES: break;
        case ESP_ERR_NVS_NEW_VERSION_FOUND: break;

        case ESP_ERR_NVS_INVALID_LENGTH: break;
        case ESP_ERR_NVS_INVALID_HANDLE: break;
        case ESP_ERR_NVS_INVALID_STATE: break;
        case ESP_ERR_NVS_TYPE_MISMATCH: break;

        case ESP_ERR_NVS_KEY_TOO_LONG: break;
        case ESP_ERR_NVS_VALUE_TOO_LONG: break;

        case ESP_ERR_NVS_NOT_FOUND:
        case ESP_ERR_NVS_NOT_INITIALIZED:
        case ESP_ERR_NVS_KEYS_NOT_INITIALIZED: break;
        case ESP_ERR_NVS_PART_NOT_FOUND:
        case ESP_ERR_NVS_READ_ONLY:
        case ESP_ERR_NVS_CORRUPT_KEY_PART: break;
        case ESP_ERR_NVS_REMOVE_FAILED:

        case ESP_ERR_NVS_WRONG_ENCRYPTION:
        case ESP_ERR_NVS_XTS_CFG_FAILED:
        case ESP_ERR_NVS_XTS_DECR_FAILED:
        case ESP_ERR_NVS_XTS_CFG_NOT_FOUND:
        case ESP_ERR_NVS_XTS_ENCR_FAILED:
        default: break;
    }
}

bool init_nvs(){
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    bool set = set_pin(BOOT_PIN, GPIO_MODE_INPUT, true, false);
    if(set) xTaskCreate(boot_handler, "boot_handler", 2048, NULL, 10, NULL);
    else err--;
    return err == ESP_OK;
}

bool nvs_write(const char *key, const char *value){
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    err += nvs_set_str(handle, key, value);
    err += nvs_commit(handle);
    nvs_close(handle);
    return err == ESP_OK;
}

char *nvs_read(const char *key){
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READONLY, &handle);
    size_t size = 0;
    err += nvs_get_str(handle, key, NULL, &size);
    if(err == ESP_OK && size != 0){
        char *buffer = malloc(size);
        err = nvs_get_str(handle, key, buffer, &size);
        nvs_close(handle);
        return buffer;
    } else {
        nvs_close(handle);
        return "";
    }
}
