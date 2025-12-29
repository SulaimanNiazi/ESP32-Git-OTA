#include "ota.h"
#include "log.h"

#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

static char buffer[OTA_MAX_LENGTH], current[32], latest[32] = "0.0.0";
static bool up_to_date = true;
static volatile size_t buffer_len = 0;

void process_section(const char *section){
    if(strstr(section, OTA_HARDWARE)){
        char *ptr = strstr(section, "\": \"") + 3;
        bool newer = false;
        for(size_t x = 0; (*(++ptr) != '\"') && (x < 32); x++){
            if(*ptr == '.'){
                if(latest[x] == '.') continue;
                else return;
            }

            if(*ptr > latest[x]) newer = true;
            if(newer) latest[x] = *ptr;
        }
        ESP_LOGI(OTA_LOG_TAG, "Supported version found: %s", latest);
    }
}

static esp_err_t event_handler(esp_http_client_event_t *event){
    if(event->event_id != HTTP_EVENT_ON_DATA) return ESP_OK;

    const char *data = event->data;
    const size_t len = event->data_len;
    for(size_t i = 0; i < len; i++){
        char c = data[i];
        if(c == ']'){
            buffer[buffer_len] = '\0';
            process_section(buffer);
            buffer_len = 0;
        }
        else if(buffer_len < OTA_MAX_LENGTH){
            buffer[buffer_len++] = c;
        }else{
            buffer_len = 0;
        }
    }
    return ESP_OK;
}

void check_ota(){
    const esp_app_desc_t *app = esp_app_get_description();
    sprintf(current, "%s", app->version);
    
    esp_http_client_config_t http_client = {
        .url                = OTA_VERSION_URL,
        .crt_bundle_attach  = esp_crt_bundle_attach,
        .event_handler      = event_handler,
    };
    esp_http_client_handle_t handle = esp_http_client_init(&http_client);
    if(!log_error(OTA_LOG_TAG, esp_http_client_perform(handle), "HTTPS Request failed")){
        for(size_t i = 0; current[i] && (i < 32); i++){
            if(current[i] != latest[i]){
                up_to_date = false;
                break;
            }
        }
    }
    esp_http_client_cleanup(handle);
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
    
    esp_http_client_config_t http_client = {
        .url                = buffer,
        .crt_bundle_attach  = esp_crt_bundle_attach,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &http_client,
    };
    esp_https_ota_handle_t handle = NULL;
    
    esp_err_t error = esp_https_ota_begin(&ota_config, &handle);
    if(!log_error(OTA_LOG_TAG, error, "Failed to begin HTTPS OTA update")){
        do{
            error = esp_https_ota_perform(handle);
        }
        while(error == ESP_ERR_HTTPS_OTA_IN_PROGRESS);
        
        if(!log_error(OTA_LOG_TAG, error, "OTA upgrade failed")){
            if(esp_https_ota_is_complete_data_received(handle)){
                if(!log_error(OTA_LOG_TAG, esp_https_ota_finish(handle), "Failed to clean up OTA upgrade and close HTTPS connection")){
                    ESP_LOGI(OTA_LOG_TAG, "OTA upgrade complete, rebooting ...");
                    esp_restart();
                }
            } else ESP_LOGE(OTA_LOG_TAG, "Incomplete OTA image received");
        }
        esp_https_ota_abort(handle);
    }
}
