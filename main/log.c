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
