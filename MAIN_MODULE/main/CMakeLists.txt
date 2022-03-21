idf_component_register(SRCS "main.c" "MODBUS_RECEIVER_task.c" "LORA_RECEIVER_task.c" "OLED_DISPLAY_task.c" "WIFI_SMARTCONFIG_task.c" "HTTP_SERVER_task.c"
                            "libraries/MODBUS_MASTER.c" "libraries/LORA.c" "libraries/modbus_params.c" "libraries/OLED.c" "libraries/font.c" "libraries/WIFI_SMARTCONFIG.c"
                            "libraries/HTTP_SERVER.c"
                    INCLUDE_DIRS ".")