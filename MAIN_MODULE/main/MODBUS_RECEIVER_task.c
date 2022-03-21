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

//Control of modbus inicialization
int mb_init_crt = 0;

//Declaration of MODBUS devices list
mb_parameter_descriptor_t* mb_device_param = NULL;


void requestParameter(char * message)
{

    ESP_LOGI(MODULE_NAME, "Requesting parameters from MODBUS device: %s", message);

	master_get_param_data(mb_device_param);
    * message = (char *)mb_device_param->param_units;
}



void modbus_receiver_task(void *pvParameters)
{

    if(mb_init_crt <= 3)
    {
        //Ckeck MODBUS system and peripherals
        modbus_receiver_check_function();

        // Inits the MODBUS module mode
        master_init();
        ESP_LOGI(MODULE_NAME, "MODBUS iniciated correctly");

        mb_init_crt++;

        //Asign the memory for the incomming MODBUS message in the heap
        char * in_message = (char *)malloc(LEN_MESSAGES_MODBUS);
        if(in_message == NULL){
            ESP_LOGE(MODULE_NAME, "%s malloc.1 failed\n", __func__);
        }

        requestParameter(*in_message);

        // Task Loop
        for (;;) {
            
            //Waiting to receive an incoming message from MODBUS.
            if (mb_device_param != NULL) {

                BaseType_t modbus_receiver_status;

                int packetSize = (char*)mb_device_param->param_size;  //Size of the modbus parameter
                in_message = (char*)mb_device_param->param_units;    //Value of the modbus parameter
                
                // SEND THE RECEIVED MESSAGE BY THE QUEUE
                modbus_receiver_status = xQueueSend(modbus_receiver_queue,(void * ) &in_message, 10/portTICK_PERIOD_MS);
                
                // Check the message has been correctly send into the queue
                if(modbus_receiver_status == pdPASS){
                    ESP_LOGI(MODULE_NAME, "MODBUS message send to the queue correctly");
                }else{
                    ESP_LOGI(MODULE_NAME, "ERROR SENDING MESSAGE");
                }

            }    
        
            // Task delay
            vTaskDelay(10 / portTICK_PERIOD_MS);

        }

        vTaskDelete(NULL);
        master_operation_func(NULL);        //Stop MODBUS operations

    }
    else
    {
        ESP_LOGI(MODULE_NAME, "Failed MODBUS iniciation");
        mb_init_crt++;
    }
}
