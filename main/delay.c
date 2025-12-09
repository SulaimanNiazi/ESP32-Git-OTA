#include "delay.h"

void delay_ms(const unsigned int ms){
    vTaskDelay(pdMS_TO_TICKS(ms));
}
