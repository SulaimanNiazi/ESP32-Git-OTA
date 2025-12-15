#include "ota.h"
#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

static char ota_buffer[OTA_BUFFER_SIZE], current_version[32], latest_version[32];
static bool up_to_date = true;

void check_ota(){
    const esp_app_desc_t *app = esp_app_get_description();
    sprintf(current_version, "%s", app->version);

    esp_http_client_config_t config = {
        .method             = HTTP_METHOD_GET,
        .url                = OTA_VERSION_URL,
        .host               = OTA_HOST,
        .port               = OTA_PORT,
        .crt_bundle_attach  = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if(esp_http_client_open(client, 0) == ESP_OK){
        if(esp_http_client_fetch_headers(client)){
            int len = esp_http_client_read(client, ota_buffer, OTA_BUFFER_SIZE);
            esp_http_client_cleanup(client);
            
            if(len){ // Should be >= 366
                for(; ota_buffer[--len] != ' ';); // Find the last ' '
                for(size_t ver_ind = 0, buf_ind = len; ota_buffer[++buf_ind] != ')'; ver_ind++){
                    latest_version[ver_ind] = ota_buffer[buf_ind];
                    if(latest_version[ver_ind] != current_version[ver_ind]) up_to_date = false;
                }
                return;
            }
        }
    }
    esp_http_client_cleanup(client);
    sprintf(latest_version, "%s", current_version);
}

char *ota_get_version(const bool latest){
    return latest? latest_version: current_version;
}

bool ota_up_to_date(){
    return up_to_date;
}

void ota_update(){
    if(up_to_date || OTA_DISABLED) return;
    
    esp_http_client_config_t client_config = {
        .url                = OTA_BIN_URL,
        .crt_bundle_attach  = esp_crt_bundle_attach,
        .timeout_ms         = 15000,
    };
    esp_https_ota_config_t ota_config = {.http_config = &client_config};
    if(esp_https_ota(&ota_config) == ESP_OK) esp_restart();
}
