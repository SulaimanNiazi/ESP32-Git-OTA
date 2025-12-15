#include "wifi.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t wifi_event_group;
volatile size_t wifi_retry_count = 0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    switch(event_id){
        // event_base = WIFI_EVENT
        case WIFI_EVENT_STA_DISCONNECTED:
            if(++wifi_retry_count > WIFI_MAX_RETRIES) break;
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;

        // event_base = IP_EVENT
        case IP_EVENT_STA_GOT_IP:
            ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data;
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            wifi_retry_count = 0;
        
        default: break;
    }
}

void init_wifi(const char *ssid, const char *password){
    wifi_event_group = xEventGroupCreate();

    // Initialize network stack
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta(); // This creates the default STA interface named "WIFI_STA_DEF".

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t config2 = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    snprintf((char*)config2.sta.ssid, 32, "%s", ssid);
    snprintf((char*)config2.sta.password, 64, "%s", password);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &config2);
    esp_wifi_start();
}

void deinit_wifi(){
    esp_wifi_stop();
    esp_wifi_deinit();
    vEventGroupDelete(wifi_event_group);
}

void wifi_await_connection(){
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(bits & WIFI_CONNECTED_BIT){
    } else {
    }
}

bool wifi_connected(){
    EventBits_t bits = xEventGroupGetBits(wifi_event_group);
    return bits & WIFI_CONNECTED_BIT;
}

esp_ip4_addr_t *wifi_get_ip_info(const size_t choice){
    static esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_get_ip_info(netif, &ip_info);
    
    switch(choice){
        case 1: return &ip_info.gw;
        case 2: return &ip_info.netmask;
        default: return &ip_info.ip;
    }
}

char *wifi_get_ip(){
    static char ip_buffer[16];
    snprintf(ip_buffer, 16, IPSTR, IP2STR(wifi_get_ip_info(0)));
    return ip_buffer;
}

char *wifi_get_gateway(){
    static char gw[16];
    snprintf(gw, 16, IPSTR, IP2STR(wifi_get_ip_info(1)));
    return gw;
}

char *wifi_get_netmask(){
    static char mask[16];
    snprintf(mask, 16, IPSTR, IP2STR(wifi_get_ip_info(2)));
    return mask;
}
