/* 
    Wifi SmartConfig
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event. Are we connected to the AP with an IP? */
const int CONNECTED_BIT = BIT0;
const int ESPTOUCH_DONE_BIT = BIT1;
const char *SMARTCONFIG_TAG = "WIFI SMARTCONFIG MODE";

void wifi_smartconfig_func(void * parm);

//Receive SSID, PASSWORD and data from WIFI communication
void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xTaskCreate(wifi_smartconfig_func, "wifi_smartconfig_func", 4096, NULL, 3, NULL);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(SMARTCONFIG_TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(SMARTCONFIG_TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(SMARTCONFIG_TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *connected_event = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, connected_event->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, connected_event->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = connected_event->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, connected_event->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, connected_event->ssid, sizeof(connected_event->ssid));
        memcpy(password, connected_event->password, sizeof(connected_event->password));
        ESP_LOGI(SMARTCONFIG_TAG, "SSID:%s", ssid);
        ESP_LOGI(SMARTCONFIG_TAG, "PASSWORD:%s", password);
        if (connected_event->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(SMARTCONFIG_TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

//Initialize WIFI configuration and start WIFI operations
void initialise_wifi(void)
{   
    ESP_ERROR_CHECK( nvs_flash_init() );

    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&wifi_init_config) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

//Set smartconfig type and connect device to an AP
void wifi_smartconfig_func(void * parm)
{
    EventBits_t event_bits;
    ESP_LOGI(SMARTCONFIG_TAG, "set smartconfig type");
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t smart_config_init = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&smart_config_init) );
    while (1) {
        event_bits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(event_bits & CONNECTED_BIT) {
            ESP_LOGI(SMARTCONFIG_TAG, "WiFi Connected to AP");
        }
        if(event_bits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(SMARTCONFIG_TAG, "WiFi smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}