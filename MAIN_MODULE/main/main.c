// Libraries Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"

// Project Includes
#include "main.h"
#include "config.h"
#include "MODBUS_RECEIVER_task.h"
#include "LORA_RECEIVER_task.h"
#include "WIFI_SMARTCONFIG_task.h"
#include "HTTP_SERVER_task.h"
#include "OLED_DISPLAY_task.h"

#define APP_NAME "TFG_APP"

// FREE RTOS DEFINITIONS
xQueueHandle display_queue = NULL;
xQueueHandle modbus_receiver_queue = NULL;
xQueueHandle lora_receiver_queue = NULL;
xQueueHandle wifi_smartconfig_queue = NULL;
xQueueHandle http_server_queue = NULL;

/*
 *  MAIN VARIABLES
 ****************************************************************************************
 */

//Declaration of communications counter
int comm_counter = 0;
int comm_crt = 1;

/*
// Declaration of semaphore to manage the access to critical resources
SemaphoreHandle_t comm_semaphore = NULL;

// A task that creates the communications semaphore.
void vTask( void * pvParameters )
{
	// Create the binary semaphore (between 0 & 1) to guard a shared resource.
	vSemaphoreCreateBinary( comm_semaphore );
	ESP_LOGE(APP_NAME, "%s semaphore value\n", (char *)comm_semaphore);
}

void delay( int msec )
{
    vTaskDelay( msec / portTICK_PERIOD_MS);
}*/

/*
 *  INIT BOARD FUNCTIONS
 ****************************************************************************************
 */

//Configure GPIO ports
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

//Control communications between system functions
void main_task( void* param )
{

	// Config Flash Pin
	gpio_num_t fp = (gpio_num_t) FLASH_PIN;
	gpio_pad_select_gpio( fp );
	gpio_set_direction( fp , GPIO_MODE_OUTPUT);

	//Asign the memory for the message in the heap (with the display message size)
	char * message = (char *)malloc(LEN_MESSAGES_OLED);
	if(message == NULL){
		ESP_LOGE(APP_NAME, "%s malloc.1 failed\n", __func__);
	}

	for ( ;; )
	{	
		char *in_MODBUS_message = NULL;					//Received MODBUS message
		char *in_lora_message = NULL;					//Received LORA message
		char *in_wifi_message = NULL;					//Received WIFI message
		//char *in_http_server_message = NULL;			//Received HTTP server message
				
		//Waiting to receive an MODBUS incoming message.
		if (xQueueReceive(modbus_receiver_queue, (void * )&in_MODBUS_message, (portTickType)portMAX_DELAY)) 
		{	
			ESP_LOGI(APP_NAME, "MODBUS MODE"); 

			//Create object to send messages to queues
			BaseType_t MODBUS_status_display;
			BaseType_t MODBUS_status_server;

			// Genarate display message
			sprintf(message, "[%d]:%s", comm_counter, in_MODBUS_message);	
						
			// SEND MESSAGE TO THE DISPLAY
			MODBUS_status_display = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			// SEND MESSAGE TO THE SERVER
			MODBUS_status_server = xQueueSend(http_server_queue, (void * ) &message, 10/portTICK_PERIOD_MS);
					
			// Check the the message has been correctly send into the display queue
			if(MODBUS_status_display == pdPASS){
			ESP_LOGI(APP_NAME, "MODBUS message send to display correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Check the the message has been correctly send into the server queue
			if(MODBUS_status_server == pdPASS){
			ESP_LOGI(APP_NAME, "MODBUS message send to server correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Update counter
			comm_counter++;

		}

		//Waiting to receive an LORA incoming message.
		if (xQueueReceive(lora_receiver_queue, (void * )&in_lora_message, (portTickType)portMAX_DELAY)) 
		{
			ESP_LOGI(APP_NAME, "LORA MODE"); 

			//Create object to send messages to queues
			BaseType_t lora_status_displey;
			BaseType_t lora_status_server;

			// Genarate display message
			sprintf( message, "[%d]:%s", comm_counter, in_lora_message);	
						
			// SEND MESSAGE TO THE DISPLAY
			lora_status_displey = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			// SEND MESSAGE TO THE SERVER
			lora_status_server = xQueueSend(http_server_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			// Check the the message has been correctly send into the queue
			if(lora_status_displey == pdPASS){
				ESP_LOGI(APP_NAME, "LORA message send to display correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Check the the message has been correctly send into the server queue
			if(lora_status_server == pdPASS){
			ESP_LOGI(APP_NAME, "LORA message send to server correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Update counter
			comm_counter++;
		
		}

		//Waiting to receive an WIFI incoming message.
		if (xQueueReceive(wifi_smartconfig_queue, (void * )&in_wifi_message, (portTickType)portMAX_DELAY)) 
		{
			ESP_LOGI(APP_NAME, "WIFI MODE"); 

			//Create object to send messages to queues
			BaseType_t wifi_status_display;
			BaseType_t wifi_status_server;

			// Genarate display message
			sprintf( message, "[%d]:%s", comm_counter, in_wifi_message);	
						
			// SEND MESSAGE TO THE DISPLAY
			wifi_status_display = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			/*// SEND MESSAGE TO THE SERVER
			wifi_status_server = xQueueSend(http_server_queue, (void * ) &message, 10/portTICK_PERIOD_MS);		¿QUE LE ENVIO AL SERVIDOR DESDE EL WIF 21.03.22DI?
			*/
			// Check the the message has been correctly send into the queue
			if(wifi_status_display == pdPASS){
				ESP_LOGI(APP_NAME, "WIFI messages send to display correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGEs");
			}

			// Check the the message has been correctly send into the server queue
			if(wifi_status_server == pdPASS){
			ESP_LOGI(APP_NAME, "WIFI message send to server correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGE");
			}

			// Update counter
			comm_counter++;

		}

		/*//Waiting to receive an HTTP SERVER incoming message.
		if (xQueueReceive(http_server_queue, (void * )&in_http_server_message, (portTickType)portMAX_DELAY)) 
		{
			ESP_LOGI(APP_NAME, "HTTP SERVER MODE"); 
	
			BaseType_t wifi_status;

			// Genarate display message
			sprintf( message, "[%d]:%s", comm_counter, in_wifi_message);	
						
			// SEND MESSAGE TO THE DISPLAY
			wifi_status = xQueueSend(display_queue, (void * ) &message, 10/portTICK_PERIOD_MS);

			// Check the the message has been correctly send into the queue
			if(wifi_status == pdPASS){
				ESP_LOGI(APP_NAME, "WIFI messages send correctly");
			}else{
				ESP_LOGI(APP_NAME, "ERROR SENDING MESSAGEs");
			}

			// Update counter
			comm_counter++;
			
		}*/

		// Blink the led
		gpio_set_level( fp, 1);
		delay(100);

		gpio_set_level( fp, 0);
		delay(1000);

		// Task delay
		vTaskDelay(50 / portTICK_PERIOD_MS);

	}

}

/*
 *  MAIN
 ****************************************************************************************
 * Use to initial configuration of the system and starting the tasks.
 */

//Create queues and init tasks
void app_main()
{

	/*** Init the FREERTOS queques ***/
	
	// Create the queque for receiving MODBUS Messages
    modbus_receiver_queue = xQueueCreate(5, sizeof(char *));
    if( modbus_receiver_queue == NULL )
	{ 
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the modbus_receiver_queue\n");

	}

	// Create the queque for receiving LORA Messages
    lora_receiver_queue = xQueueCreate(5, sizeof(char *));
    if( lora_receiver_queue == NULL )
	{ 
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the lora_receiver_queue\n");

	}

	// Create the queque for receiving WIFI Messages
    wifi_smartconfig_queue = xQueueCreate(5, sizeof(char *));
    if( wifi_smartconfig_queue == NULL )
	{ 
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the wifi_smartconfig_queue\n");

	}

	// Create the queque for sending the messages to HTTP server
    http_server_queue = xQueueCreate(5, sizeof(char *));								
    if( http_server_queue == NULL )
	{
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the http_server_queue\n");

	}

	// Create the queque for sending the messages to display
    display_queue = xQueueCreate(5, sizeof(char *));
    if( display_queue == NULL )
	{
        // There was not enough heap memory space available to create the message buffer. 
        ESP_LOGE(APP_NAME, "Not enough memory to create the display_queue\n");

	}


	/*** Init the system tasks ***/

	xTaskCreate(main_task, "main_task", 10000, NULL, 10, NULL);

	xTaskCreate(modbus_receiver_task, "modbus_receiver_task", 20000, NULL, 8, NULL);

	xTaskCreate(lora_receiver_task, "lora_receiver_task", 20000, NULL, 7, NULL);

	xTaskCreate(wifi_smartconfig_task, "wifi_smartconfig_task", 20000, NULL, 9, NULL);

	xTaskCreate(http_server_task, "http_server_task", 20000, NULL, 1, NULL);

	xTaskCreate(display_task, "display_task", 2000, NULL, 2, NULL);
}

