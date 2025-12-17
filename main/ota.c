#include "ota.h"
#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

static char ota_buffer[OTA_BUFFER_SIZE], current_version[32], latest_version[32];
static bool up_to_date = true;
static const esp_http_client_config_t http_client = {
    .url                = OTA_BIN_URL,
    .crt_bundle_attach  = esp_crt_bundle_attach,
};

void check_ota(){
    const esp_app_desc_t *app = esp_app_get_description();
    sprintf(current_version, "%s", app->version);
    esp_http_client_handle_t handle = esp_http_client_init(&http_client);

    bool failed = true;
    if(esp_http_client_open(handle, 0) == ESP_OK){
        if(esp_http_client_fetch_headers(handle)){
            int len = esp_http_client_read(handle, ota_buffer, OTA_BUFFER_SIZE);
            failed = len < OTA_VERSION_INDEX;
        }
    }
    esp_http_client_cleanup(handle);
    
    if(failed) sprintf(latest_version, "%s", current_version);
    else{
        for(size_t vi = 0, bi = OTA_VERSION_INDEX; ota_buffer[++bi]; vi++){
            latest_version[vi] = ota_buffer[bi];
            if(latest_version[vi] != current_version[vi]) up_to_date = false;
        }
    }
}

char *ota_get_version(const bool latest){
    return latest? latest_version: current_version;
}

bool ota_up_to_date(){
    return up_to_date;
}

void ota_update(){
    if(up_to_date || OTA_DISABLED) return;
    
    esp_https_ota_config_t ota_config = {.http_config = &http_client};
    if(esp_https_ota(&ota_config) == ESP_OK) esp_restart();
}
