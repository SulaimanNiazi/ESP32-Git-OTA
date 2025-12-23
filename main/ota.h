#pragma once

#define OTA_DISABLED        false
#define OTA_HARDWARE        "ESP32 WROOM 32"
#define OTA_FIRMWARE_URL    "https://raw.githubusercontent.com/SulaimanNiazi/ESP32-Git-OTA/refs/heads/main/firmware"
#define OTA_VERSION_URL     "https://raw.githubusercontent.com/SulaimanNiazi/ESP32-Git-OTA/refs/heads/main/version.json"
#define OTA_MAX_LENGTH      200
#define OTA_LOG_TAG         "OTA"

#include "esp_system.h"

void check_ota();
char *ota_get_version(const bool latest);
bool ota_up_to_date();
void ota_update();
