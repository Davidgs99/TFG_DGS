#ifndef CONFIG

#define CONFIG

/*** Board Configuration ***/
#define PIN_NUM_MISO 	19
#define PIN_NUM_MOSI 	27
#define PIN_NUM_CLK  	5
#define PIN_NUM_CS   	18
#define PIN_NUM_DIO		26
#define RESET_PIN  		23

#define SENDER_RECEIVER_PIN	12
#define	FLASH_PIN			25


/*** OLED DISPLAY System config ***/
#define LEN_MESSAGES_OLED 20 


/*** MODBUS protocol System config ***/
#define LEN_MESSAGES_MODBUS 20              //CHECK IT  
/*#define MB_UART_PORT_NUM 2 
#define MB_UART_BAUD_RATE 115200 
#define MB_UART_RXD 0               //CHECK IT 
#define MB_UART_TXD 0               //CHECK IT 
#define MB_UART_RTS 0               //CHECK IT  */


/*** LORA protocol System config ***/
#define LEN_MESSAGES_LORA 20                



/*** WIFI protocol System config ***/
#define LEN_MESSAGES_WIFI 64              //CHECK IT  


#endif
