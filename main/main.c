// Libraries Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"

// MODBUS Libraries Includes
#include "mbcontroller.h"
#include "esp_modbus_common.h"
#include "esp_modbus_master.h"

// Project Includes
#include "main.h"
#include "config.h"
#include "MODBUS_RECEIVER_task.h"
#include "OLED_DISPLAY_task.h"


#define APP_NAME "TFG_APP"

//FREE RTOS DEFINITIONS
xQueueHandle display_queue = NULL;
xQueueHandle modbus_receiver_queue = NULL;

int _counter = 0;
int comm_flat = 0;

void delay( int msec )
{
    vTaskDelay( msec / portTICK_PERIOD_MS);
}

/*
 *  INIT BOARD FUNCTIONS
 ****************************************************************************************
 */

bool getConfigPin()
{
    gpio_config_t io_conf;
    gpio_num_t pin = (gpio_num_t) SENDER_RECEIVER_PIN;

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << pin );
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    gpio_config(&io_conf);

    return gpio_get_level( pin ) == 0;
}



/*
 *  MAIN TASK 
 ****************************************************************************************
 * Main task area. 
 */

//void app_main()
void main_task( void* param )
{

	// Config Flash Pin
	gpio_num_t fp = (gpio_num_t) FLASH_PIN;
	gpio_pad_select_gpio( fp );
	gpio_set_direction( fp , GPIO_MODE_OUTPUT);

	//Asign the memory for the message in the heap
	char * message = (char *)malloc(LEN_MESSAGES_MODBUS);
	if(message == NULL){
		ESP_LOGE(APP_NAME, "%s malloc.1 failed\n", __func__);
	}

	for ( ;; )
	{
		BaseType_t status;

		// Update the message
		sprintf( message, "hello: [%d]", _counter);	
		
		// SEND MESSAGE TO MODBUS SENDER TASK
		//Send the pointer to the message (Reduces memmory use by the queue)
		status = xQueueSend(modbus_receiver_queue, (void * ) &message, 10/portTICK_PERIOD_MS);
		// Check the the message has been correctly send into the queue
		if(status == pdPASS){
			ESP_LOGI(APP_NAME, "Message send correctly");
		}else{
			ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
		}

		// SEND MESSAGE TO THE DISPLAY
		status = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);
		// Check the the message has been correctly send into the queue
		if(status == pdPASS){
			ESP_LOGI(APP_NAME, "Message send correctly");
		}else{
			ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
		}

		// Blink the led
		gpio_set_level( fp, 1);
		delay(100);

		gpio_set_level( fp, 0);
		delay(1000);

		// Update counter
		_counter++;

		// Task delay
		vTaskDelay(50 / portTICK_PERIOD_MS);

	}

}

/*
 *  MAIN
 ****************************************************************************************
 * Use to initial configuration of the system and starting the tasks.
 */

void app_main()
{

	/*** Init the FREERTOS queques ***/
	
	// Create the queque for receiving MODBUS Messages
    modbus_receiver_queue = xQueueCreate(5, sizeof(char *));
    if( modbus_receiver_queue == NULL ) 
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the modbus_receiver_queue\n");


	// Create the queque for sending the messages to display
    display_queue = xQueueCreate(5, sizeof(char *));
    if( display_queue == NULL )
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the display_queue\n");



	/*** Init the system tasks ***/

	xTaskCreate(main_task, "main_task", 10000, NULL, 10, NULL);

	xTaskCreate(modbus_receiver_task, "modbus_receiver_task", 2000, NULL, 5, NULL);

	xTaskCreate(display_task, "display_task", 2000, NULL, 1, NULL);
}

