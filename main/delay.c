#include "delay.h"
#include "freertos/FreeRTOS.h" // for pdMS_TO_TICKS
#include "freertos/task.h"

void delay_ms(const unsigned int ms){
    vTaskDelay(pdMS_TO_TICKS(ms));
}
