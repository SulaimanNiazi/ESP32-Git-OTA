#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void init_log(size_t count, ...){
    esp_log_level_set("*", ESP_LOG_NONE);
    va_list vl;
    va_start(vl, count);
    while(count--){
        char *tag = va_arg(vl, char*);
        printf("%s", tag);
        esp_log_level_set(tag, ESP_LOG_INFO);
    }
    va_end(vl);
}

bool log_error(const char *tag, const esp_err_t error, const char *detail){
    if(error == ESP_OK) return false;
    ESP_LOGE(tag, "%s: %s.", esp_err_to_name(error), detail);
    return true;
}
