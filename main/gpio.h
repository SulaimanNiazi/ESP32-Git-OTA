#pragma once

#include "driver/gpio.h"

bool set_pin(const unsigned int pin, const unsigned int mode, bool pullup, bool pulldown);
