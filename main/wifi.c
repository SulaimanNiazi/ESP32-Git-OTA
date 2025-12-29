#include "wifi.h"
#include "log.h"

#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t wifi_event_group;
volatile static size_t retry_count = 0;
static char ip[16] = "", gw[16] = "", netmask[16] = "";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    if(event_base == WIFI_EVENT){
        switch(event_id){
            case WIFI_EVENT_STA_DISCONNECTED: if(++retry_count > WIFI_MAX_RETRIES){
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                ip[0] = gw[0] = netmask[0] = 0;
            }
            [[fallthrough]];
            case WIFI_EVENT_STA_START: esp_wifi_connect();
            default: break;
        }
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_ip_info_t ip_info;
        if(loge_success(WIFI_LOG_TAG, esp_netif_get_ip_info(netif, &ip_info), "Failed to get interface's IP info")){
            snprintf(ip,      16, IPSTR, IP2STR(&ip_info.ip));
            snprintf(gw,      16, IPSTR, IP2STR(&ip_info.gw));
            snprintf(netmask, 16, IPSTR, IP2STR(&ip_info.netmask));

            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            retry_count = 0;
        }
    }
}

void init_wifi(const char *ssid, const char *password){
    wifi_event_group = xEventGroupCreate();

    // Initialize network stack
    loge_success(WIFI_LOG_TAG, esp_netif_init(), "Failed to initialize the underlying TCP/IP stack");
    esp_err_t error = esp_event_loop_create_default();
    if(error == ESP_ERR_INVALID_STATE){
        ESP_LOGE(WIFI_LOG_TAG, "Default event loop has already been created.");
    }else{
        loge_success(WIFI_LOG_TAG, error, "Failed to create default event loop.");
    }

    esp_netif_create_default_wifi_sta(); // This creates the default STA interface named "WIFI_STA_DEF".

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    loge_success(WIFI_LOG_TAG, esp_wifi_init(&config), "Failed to initialize Wifi");

    loge_success(WIFI_LOG_TAG, esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL), "Failed to register the event handler instance to the default loop");
    loge_success(WIFI_LOG_TAG, esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL), "Failed to register the event handler instance to the default loop");

    wifi_config_t config2 = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    snprintf((char*)config2.sta.ssid, 32, "%s", ssid);
    snprintf((char*)config2.sta.password, 64, "%s", password);

    loge_success(WIFI_LOG_TAG, esp_wifi_set_mode(WIFI_MODE_STA), "Failed to set STA mode");
    loge_success(WIFI_LOG_TAG, esp_wifi_set_config(WIFI_IF_STA, &config2), "Failed to configure STA");
    loge_success(WIFI_LOG_TAG, esp_wifi_start(), "Failed to initialize Wifi");
}

void deinit_wifi(){
    loge_success(WIFI_LOG_TAG, esp_wifi_stop(), "Failed to stop Wifi");
    loge_success(WIFI_LOG_TAG, esp_wifi_deinit(), "Failed to de-initialize Wifi");
    vEventGroupDelete(wifi_event_group);
}

void wifi_await_connection(){
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(bits & WIFI_CONNECTED_BIT){
    } else {
    }
}

bool wifi_connected(){
    EventBits_t bits = xEventGroupGetBits(wifi_event_group);
    return bits & WIFI_CONNECTED_BIT;
}

char *wifi_get_ip(){
    return ip;
}

char *wifi_get_gateway(){
    return gw;
}

char *wifi_get_netmask(){
    return netmask;
}
