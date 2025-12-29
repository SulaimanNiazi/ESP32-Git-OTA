#pragma once

#include "esp_log.h"
#include "esp_err.h"

void init_log(size_t count, ...);
bool loge_success(const char *tag, const esp_err_t error, const char *detail);
