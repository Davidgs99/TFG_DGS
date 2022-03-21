#include "string.h"
#include "stdlib.h"
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
#include "esp_now.h"
#include "esp_smartconfig.h"

#include "libraries/WIFI_SMARTCONFIG.h"
#include "config.h"
#include "main.h"

#define MODULE_NAME "WIFI_SMARTCONFIG_MOD"

extern xQueueHandle wifi_smartconfig_queue;

//Receive WIFI messages and send them to the WIFI queue
void wifi_smartconfig_task(void *pvParameters)
{

    esp_event_base_t base_event = NULL;
    int32_t id_event = 0;
    char *data_event = NULL; 

    //Asign the memory for the incomming message in the heap
	char * in_message = (char *)malloc(LEN_MESSAGES_WIFI);
	if(in_message == NULL){
		ESP_LOGE(MODULE_NAME, "%s malloc.1 failed\n", __func__);
	}

    // Inits the WIFI manager functions
    ESP_LOGI(MODULE_NAME, "init smartconfig");
    wifi_smartconfig_func(in_message);

    // Task loop
    for (;;) {

        // Check if we have any WIFI event
        if(base_event != NULL){

            BaseType_t wifi_smartconfig_status;

            event_handler(in_message, base_event, id_event, data_event); 

            smartconfig_event_got_ssid_pswd_t *connected_event = (smartconfig_event_got_ssid_pswd_t *)data_event;
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
            in_message = (char*)esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data));

            // SEND THE RECEIVED MESSAGE BY THE QUEUE
            wifi_smartconfig_status = xQueueSend(wifi_smartconfig_queue, (void * ) &in_message, 10/portTICK_PERIOD_MS);

            // Check the the message has been correctly send into the queue
            if(wifi_smartconfig_status == pdPASS){
                ESP_LOGI(MODULE_NAME, "WIFI message send to the queue correctly");
            }else{
                ESP_LOGI(MODULE_NAME, "ERROR SENDING MESSAGE");
            }

        }

        // Task delay
		vTaskDelay(10 / portTICK_PERIOD_MS);

    }

    vTaskDelete(NULL);
}
