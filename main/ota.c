#include "ota.h"
#include "log.h"

#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

static char buffer[OTA_BUFFER_SIZE], current_version[32], latest_version[32];
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
    esp_err_t result = esp_http_client_open(handle, 0);
    if(result == ESP_OK){
        result = esp_http_client_fetch_headers(handle);
        if(result){
            result = esp_http_client_read(handle, buffer, OTA_BUFFER_SIZE);
            failed = result < OTA_VERSION_INDEX;
        }
        switch(result){
            case 0:
                ESP_LOGW(OTA_LOG_TAG, "The data stream either doesn't contain content-length header or has chunked encoding.");
                result = ESP_FAIL;
                break;
            case -ESP_ERR_HTTP_EAGAIN: result = ESP_ERR_HTTP_EAGAIN; break;
            default: break;
        }
    }
    esp_http_client_cleanup(handle);
    
    if(failed){
        ESP_LOGE(OTA_LOG_TAG, "Failed to read bin file from github: %s.", esp_err_to_name(result));
        sprintf(latest_version, "%s", current_version);
    }else{
        for(size_t bi = OTA_VERSION_INDEX, vi = 0; buffer[++bi]; vi++){
            latest_version[vi] = buffer[bi];
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
    if(up_to_date || OTA_DISABLED){
        ESP_LOGW(OTA_LOG_TAG, "%s.", up_to_date? "Version is already up to date": "OTA is disabled");
        return;
    }
    
    esp_https_ota_config_t ota_config = {.http_config = &http_client};
    if(!log_error(OTA_LOG_TAG, esp_https_ota(&ota_config), "OTA Failed")){
        ESP_LOGI(OTA_LOG_TAG, "OTA Complete. Restarting...");
        esp_restart();
    }
}
