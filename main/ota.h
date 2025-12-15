#pragma once

#define OTA_DISABLED     false
#define OTA_VERSION_URL "https://raw.githubusercontent.com/SulaimanNiazi/ESP32-Git-OTA/refs/heads/main/CMakeLists.txt"
#define OTA_BIN_URL     "https://raw.githubusercontent.com/SulaimanNiazi/ESP32-Git-OTA/refs/heads/main/build/Git_OTA.bin"
#define OTA_HOST        "raw.githubusercontent.com"
#define OTA_PORT        443
#define OTA_BUFFER_SIZE 500

#include "esp_system.h"

void check_ota();
char *ota_get_version(const bool latest);
bool ota_up_to_date();
void ota_update();
