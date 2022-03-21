#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include "esp_http_server.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"

#include "libraries/HTTP_SERVER.h"
#include "main.h"

#define MODULE_NAME "HTTP_SERVER_TASK"
    
//extern xQueueHandle http_server_LORA_queue;
//extern xQueueHandle http_server_MODBUS_queue;
extern xQueueHandle http_server_queue;
char queue_message;

//Handler to send data to server from module                REVISAR COMO CAMBIAR LOS DATOS DE user_ctx 
const httpd_uri_t data = {                                 
    .uri       = "/data",
    .method    = HTTP_GET,
    .handler   = data_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "data handler"
};

httpd_handle_t server = NULL;               //Create a server which will show sensors measures

httpd_req_t *request;                       //Create a request to send data to the server
    
//Start server and upload messages 
void http_server_task(void *pvParameters)
{

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());       //Create a event loop to register event handlers 

    /* Register event handlers to stop the server when Wi-Fi is disconnected and re-start it upon connection */
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));                  //If module receive an IP address it starts the webserver
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));     //When lose WIFI connection it disconnects the webserver 

    /* Start the server for the first time */
    server = start_webserver();  

    if (xQueueReceive(http_server_queue, (void * )&queue_message, (portTickType)portMAX_DELAY)){            //If receive any message from server queue, send it to server.
        
        request->user_ctx = queue_message;                                                                  //Introduce server queue message into request user data
        data_get_handler(request);
        ESP_LOGI(MODULE_NAME, "Data message sent successfully");

    }

   /* Stop the server */
   stop_webserver(server); 

}