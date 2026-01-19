#include "ota.h"
#include "log.h"

#include "esp_app_desc.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

#include <cJSON.h>

static char buffer[OTA_MAX_LENGTH], current[32], latest[32], sha256[64];
static bool up_to_date = true;
static volatile bool manifest_error;
static volatile size_t buffer_len = 0;
static const char *hardware = HARDWARE;
static const esp_app_desc_t *app;

void init_ota(){
    app = esp_app_get_description();
    strcpy(current, app->version);
}

static esp_err_t event_handler(esp_http_client_event_t *event){
    if(event->event_id != HTTP_EVENT_ON_DATA) return ESP_OK;

    const char *data = event->data;
    const size_t len = event->data_len;
    for(size_t i = 0; i < len; i++){
        switch(data[i]){
            case ' ': case '\n': continue;

            default:
                if(buffer_len < OTA_MAX_LENGTH){
                    buffer[buffer_len] = data[i];

                    if(buffer_len++) if(data[i] == '}'){
                        if(strstr(buffer, hardware)){
                            char *substr = strstr(buffer, ":{") + 1;
                            if(substr){
                                cJSON *root = cJSON_Parse(substr);
                                if(root){
                                    cJSON *version = cJSON_GetObjectItem(root, "version"), *sha = cJSON_GetObjectItem(root, "sha256");
                                    
                                    if(cJSON_IsString(version) && cJSON_IsString(sha)){
                                        strcpy(latest, version->valuestring);
                                        strncpy(sha256, sha->valuestring, sizeof(sha256));
                                    }else{
                                        ESP_LOGE(OTA_LOG_TAG, "Values extracted from json are not strings");
                                        manifest_error = true;
                                    }

                                    cJSON_Delete(root);
                                }else{
                                    ESP_LOGE(OTA_LOG_TAG, "Failed to parse json manifest: %s", cJSON_GetErrorPtr()); // perhaps due to low memory
                                    manifest_error = true;
                                    // ESP_LOGI(OTA_LOG_TAG, "Reading json file manually...");
                                    // if(strstr(buffer, hardware)){
                                    //     char *read = strstr(buffer, "\":\"") + 2;
                                    //     for(size_t write = 0; (*(++read) != '\"') && (write < 32); latest[write++] = *read);
                                    // }
                                }
                                
                                return ESP_FAIL; // End request
                            }
                        }
                        goto clear;
                    }
                }else{
                    clear:
                    buffer_len = 0;
                    buffer[0] = '\0';
                }
        }
    }
    return ESP_OK;
}

void check_ota(){
    esp_http_client_config_t client_config = {
        .url                = OTA_VERSION_URL,
        .crt_bundle_attach  = esp_crt_bundle_attach,
        .event_handler      = event_handler,
    };
    esp_http_client_handle_t client_handle = esp_http_client_init(&client_config);
    
    manifest_error = false;
    esp_http_client_perform(client_handle);
    if(manifest_error){
        ESP_LOGE(OTA_LOG_TAG, "HTTPS request failed to read manifest");
        latest[0] = 0;
    }else{
        for(size_t i = 0; current[i] && (i < 32); i++){
            if(current[i] != latest[i]){
                up_to_date = false;
                break;
            }
        }
    }
    esp_http_client_cleanup(client_handle);
}

const char *ota_get_project(){
    return app->project_name;
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

    snprintf(buffer, OTA_MAX_LENGTH, "%s%s.bin", OTA_RELEASE_URL, hardware);
    ESP_LOGI(OTA_LOG_TAG, "Generated link: %s", buffer);

    esp_http_client_config_t client_config = {
        .url                = buffer,
        .crt_bundle_attach  = esp_crt_bundle_attach,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &client_config,
    };
    
    esp_https_ota_handle_t ota_handle;
    esp_err_t error = esp_https_ota_begin(&ota_config, &ota_handle);
    if(loge_success(OTA_LOG_TAG, error, "Failed to begin HTTPS OTA update")){
        esp_app_desc_t image;
        if(loge_success(OTA_LOG_TAG, esp_https_ota_get_img_desc(ota_handle, &image), "Failed to get image description")){
            ESP_LOGI(OTA_LOG_TAG, "Image project name: %s",   image.project_name);
            ESP_LOGI(OTA_LOG_TAG, "Image version: %s",        image.version);
            ESP_LOGI(OTA_LOG_TAG, "Image compile date: %s",   image.date);
            ESP_LOGI(OTA_LOG_TAG, "Image compile time: %s",   image.time);

            if(strcmp(app->project_name, image.project_name)){
                ESP_LOGE(OTA_LOG_TAG, "Image's project name (%s) does not match the current project name (%s)", image.project_name, app->project_name);
                return;
            }

            ESP_LOGI(OTA_LOG_TAG, "SHA256:");
            char byte[4];
            for(size_t i = 0; i < 64; i+=2){
                snprintf(byte, sizeof(byte), "%02x", image.app_elf_sha256[i/2]);
                printf("%s", byte);
                if((sha256[i] != byte[0])||(sha256[i+1] != byte[1])){
                    printf("\r\n");
                    ESP_LOGE(OTA_LOG_TAG, "SHA256 mismatch");
                    return;
                }
            }
            printf("\r\n");
        }

        int loaded, total = esp_https_ota_get_image_size(ota_handle);
        if(total == -1){
            ESP_LOGE(OTA_LOG_TAG, "Cannot get image size.");
            return;
        } else {
            ESP_LOGI(OTA_LOG_TAG, "Total image size: %d bytes", total);
        }

        float prev = 0, now;
        do{
            error = esp_https_ota_perform(ota_handle);
            loaded = esp_https_ota_get_image_len_read(ota_handle);
            if(loaded != -1){
                now = 100*(float)loaded/(float)total;
                if(now - prev >= 0.01){
                    ESP_LOGI(OTA_LOG_TAG, "Updating... %.2f %% (%d/%d)", now, loaded, total);
                    prev = now;
                }
            }
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
