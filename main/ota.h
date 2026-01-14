#pragma once

#define OTA_DISABLED        false
#define OTA_RELEASE_URL     "https://raw.githubusercontent.com/SulaimanNiazi/ESP32-Git-OTA-Public/refs/heads/main/firmwares/"
#define OTA_VERSION_URL     "https://raw.githubusercontent.com/SulaimanNiazi/ESP32-Git-OTA-Public/refs/heads/main/manifest.json"
#define OTA_MAX_LENGTH      150
#define OTA_LOG_TAG         "OTA"
#define OTA_VERSION_INDEX   47

#include "esp_system.h"

void check_ota();
char *ota_get_version(const bool latest);
bool ota_up_to_date();
void ota_update();
