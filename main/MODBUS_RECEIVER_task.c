#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"

#include "libraries/MODBUS_MASTER.h"
#include "config.h"
#include "main.h"


#define MODULE_NAME "MODBUS_RECEIVER_MOD"

extern xQueueHandle modbus_receiver_queue;

mb_parameter_descriptor_t* mb_device_param = NULL;


void requestParameter(char * message)
{

    ESP_LOGI(MODULE_NAME, "Requesting parameters from MODBUS device: %s", message);

	master_get_param_data(mb_device_param);
    * message = (char)mb_device_param->param_key;
}


void modbus_receiver_task(void *pvParameters)
{

    char *in_message ;

    // Ckeck the initialization of device peripheral and objects
    modbus_sender_check_function();

    // Inits the MODBUS module mode
    master_init();
    
    
    // Task Loop
    for (;;) {
        //Waiting to receive an incoming message from MODBUS.
        if (xQueueReceive(modbus_receiver_queue, (void * )&in_message, (portTickType)portMAX_DELAY)) {

            requestParameter(in_message);

        }

        // Task delay
		vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
