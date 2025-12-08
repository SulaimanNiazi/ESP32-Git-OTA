#include "gpio.h"

bool set_pin(const unsigned int pin, const unsigned int mode){
    const gpio_config_t config = {
        .pin_bit_mask   = 1ULL << pin,
        .mode           = mode
    };
    return gpio_config(&config);
}
