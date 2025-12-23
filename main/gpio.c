#include "gpio.h"
#include "log.h"

bool set_pin(const unsigned int pin, const unsigned int mode, bool pullup, bool pulldown){
    if(pullup && pulldown) pulldown = pullup = false;
    const gpio_config_t config = {
        .pin_bit_mask   = 1ULL << pin,
        .mode           = mode,
        .pull_up_en     = pullup? GPIO_PULLUP_ENABLE: GPIO_PULLUP_DISABLE,
        .pull_down_en   = pulldown? GPIO_PULLDOWN_ENABLE: GPIO_PULLDOWN_DISABLE,
    };
    return !log_error(GPIO_LOG_TAG, gpio_config(&config), "Failed to configure GPIO");
}
