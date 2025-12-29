#include "ota.h"
#include "log.h"

#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

static char buffer[OTA_MAX_LENGTH], current[32], latest[32] = "0.0.0";
static bool up_to_date = true;
static volatile size_t buffer_len = 0;
static esp_https_ota_handle_t ota_handle = NULL;

void process_section(const char *section){
    if(strstr(section, OTA_HARDWARE)){
        char *read = strstr(section, "\": \"") + 3;
        bool newer = false;
        for(size_t write = 0; (*(++read) != '\"') && (write < 32); write++){
            if(*read == '.'){
                if(latest[write] == '.') continue;
                else return;
            }

            if(*read > latest[write]) newer = true;
            if(newer) latest[write] = *read;
        }
        ESP_LOGI(OTA_LOG_TAG, "Supported version found: %s", latest);
    }
}

static esp_err_t event_handler(esp_http_client_event_t *event){
    if(event->event_id != HTTP_EVENT_ON_DATA) return ESP_OK;

    const char *data = event->data;
    const size_t len = event->data_len;
    for(size_t i = 0; i < len; i++){
        if(data[i] == ']'){
            buffer[buffer_len] = '\0';
            process_section(buffer);
            buffer_len = 0;
        }
        else if(buffer_len < OTA_MAX_LENGTH){
            buffer[buffer_len++] = data[i];
        }else{
            buffer_len = 0;
        }
    }
    return ESP_OK;
}

void check_ota(){
    const esp_app_desc_t *app = esp_app_get_description();
    sprintf(current, "%s", app->version);
    
    esp_http_client_config_t client_config = {
        .url                = OTA_VERSION_URL,
        .crt_bundle_attach  = esp_crt_bundle_attach,
        .event_handler      = event_handler,
    };
    esp_http_client_handle_t client_handle = esp_http_client_init(&client_config);
    
    bool error = true;
    if(loge_success(OTA_LOG_TAG, esp_http_client_set_url(client_handle, OTA_VERSION_URL), "Failed to set URL to get versions")){
        if(loge_success(OTA_LOG_TAG, esp_http_client_perform(client_handle), "HTTPS Request failed")){
            error = false;
            for(size_t i = 0; current[i] && (i < 32); i++){
                if(current[i] != latest[i]){
                    up_to_date = false;
                    break;
                }
            }
        }
    }
    if(error){
        latest[0] = 0;
    }
    esp_http_client_cleanup(client_handle);
}

char *ota_get_version(const bool Latest){
    return Latest? latest: current;
}

bool ota_up_to_date(){
    return up_to_date;
}

void ota_update(){
    if(up_to_date || OTA_DISABLED){
        ESP_LOGW(OTA_LOG_TAG, "%s.", up_to_date? "Version is already up to date": "OTA is disabled");
        return;
    }

    snprintf(buffer, OTA_MAX_LENGTH, "%s/%s.bin", OTA_FIRMWARE_URL, latest);
    ESP_LOGI(OTA_LOG_TAG, "Generated link: %s", buffer);
    esp_http_client_config_t client_config = {
        .url                = buffer,
        .crt_bundle_attach  = esp_crt_bundle_attach,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &client_config,
    };
    
    esp_err_t error = esp_https_ota_begin(&ota_config, &ota_handle);
    if(loge_success(OTA_LOG_TAG, error, "Failed to begin HTTPS OTA update")){
        do{
            error = esp_https_ota_perform(ota_handle);
        }
        while(error == ESP_ERR_HTTPS_OTA_IN_PROGRESS);
        
        if(loge_success(OTA_LOG_TAG, error, "OTA upgrade failed")){
            if(esp_https_ota_is_complete_data_received(ota_handle)){
                loge_success(OTA_LOG_TAG, esp_https_ota_finish(ota_handle), "Failed to clean up OTA upgrade and close HTTPS connection");
                ESP_LOGI(OTA_LOG_TAG, "OTA update complete. Restarting Device ...");
                esp_restart();
            } else ESP_LOGE(OTA_LOG_TAG, "Incomplete OTA image received");
        }
        loge_success(OTA_LOG_TAG, esp_https_ota_abort(ota_handle), "Failed to free OTA context and close HTTPS connection");
    }
}
