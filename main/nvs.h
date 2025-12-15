#pragma once

#include "delay.h"

#define NAMESPACE      "OTA"
#define BOOT_PIN       0
#define NVS_MAX_LENGTH 1024

bool init_nvs();
bool nvs_write(const char *key, const char *value);
char *nvs_read(const char *key);
