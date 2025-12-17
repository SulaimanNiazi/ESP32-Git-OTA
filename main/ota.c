#include "ota.h"
#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

static char ota_buffer[OTA_BUFFER_SIZE], current_version[32], latest_version[32];
static bool up_to_date = true;

esp_http_client_config_t get_client_config(const char *url){
    esp_http_client_config_t config = {
        .url                = url,
        .crt_bundle_attach  = esp_crt_bundle_attach,
    };
    return config;
}

void check_ota(){
    const esp_app_desc_t *app = esp_app_get_description();
    sprintf(current_version, "%s", app->version);

    esp_http_client_config_t config = get_client_config(OTA_VERSION_URL);
    esp_http_client_handle_t handle = esp_http_client_init(&config);
    if(esp_http_client_open(handle, 0) == ESP_OK){
        if(esp_http_client_fetch_headers(handle)){
            int len = esp_http_client_read(handle, ota_buffer, OTA_BUFFER_SIZE);
            esp_http_client_cleanup(handle);
            
            if(len){ // Should be c.366
                while(ota_buffer[--len] != ' ');
                while(ota_buffer[--len] != ' ');
                for(size_t ver_ind = 0, buf_ind = len; ota_buffer[++buf_ind] != ')'; ver_ind++){
                    latest_version[ver_ind] = ota_buffer[buf_ind];
                    if(latest_version[ver_ind] != current_version[ver_ind]) up_to_date = false;
                }
                return;
            }
        }
    }
    esp_http_client_cleanup(handle);
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
    
    esp_http_client_config_t client_config = get_client_config(OTA_BIN_URL);
    esp_https_ota_config_t ota_config = {.http_config = &client_config};
    if(esp_https_ota(&ota_config) == ESP_OK) esp_restart();
}
