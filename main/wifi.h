#ifndef WIFI_H
#define WIFI_H

#define WIFI_MAX_RETRIES 100

#include "stdbool.h"

void init_wifi(const char* ssid, const char* password);
void deinit_wifi();
bool wifi_connected();
char* wifi_get_ip();
void wifi_await_connection();

#endif