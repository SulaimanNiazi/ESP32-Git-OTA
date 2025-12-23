#pragma once

#define WIFI_MAX_RETRIES    10
#define WIFI_LOG_TAG        "WIFI"

#include "esp_wifi.h"

void init_wifi(const char *ssid, const char *password);
void deinit_wifi();
void wifi_await_connection();
bool wifi_connected();
char *wifi_get_ip();
char *wifi_get_gateway();
char *wifi_get_netmask();
