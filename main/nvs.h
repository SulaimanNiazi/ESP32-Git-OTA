#pragma once

#include "delay.h"

#define NAMESPACE       "OTA"
#define NVS_LOG_TAG     "NVS"
#define BOOT_PIN        0
#define NVS_MAX_LENGTH  1024

void init_nvs();
bool nvs_write(const char *key, const char *value);
char *nvs_read(const char *key);
