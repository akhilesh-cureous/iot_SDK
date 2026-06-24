#include"app_main.h"
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include<string.h>

#include "esp32_at.h"
#include "esp32_at_io.h"

extern UART_HandleTypeDef huart7;
extern DMA_HandleTypeDef hdma_uart7_rx;


extern void MPU_Config(void);
extern void SystemClock_Config(void);



int main()
{
	MPU_Config();

	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();

	MX_DMA_Init();
	MX_UART8_Init();
	MX_UART7_Init();

	if(esp32_init() != ESP32_OK){UART8_SendString("initialization failed\r\n");}
	else {UART8_SendString("initialization successful\r\n");}

	if(esp32_cwstate() != CONNECTED)
	{
		if(esp32_join_ap((uint8_t*)SSID, (uint8_t *)PASSWORD) != ESP32_OK){
			UART8_SendString("AP not Connected\r\n");
		}
		else
		{
			UART8_SendString("AP CONNECTED\r\n");

		}

	}else{
		UART8_SendString("Already connected\r\n");
	}


	if(esp32_config_sntp(UTC_OFFSET) != ESP32_OK){ UART8_SendString("SNTP time not configured\r\n");}
	else {UART8_SendString("SNTP TIME CONFIGURED\r\n");}

	if(MQTT_conn_state() == MQTT_CONN_DISCONNECTED)
	{
		if(esp32_config_mqtt() != ESP32_OK)
		{
			UART8_SendString("MQTT NOT CONNECTED\r\n");
		}
		else
		{
			UART8_SendString("MQTT connected\r\n");
		}
	}
	else
	{
		UART8_SendString("MQTT Already connected\r\n");
	}


	if(esp32_mqtt_subscribe("device/commands") == ESP32_OK) { UART8_SendString("Subscribed to device/commands\r\n");}
	else {UART8_SendString("Subscription failed\r\n"); }

	while(1)
 	{
    	esp32_wifi_status_t status;

    	uint8_t start_tick = HAL_GetTick();

    	if(HAL_GetTick() - start_tick  >= 10)
    	{
    		status = esp32_cwstate();
    	}

		switch(status)
		{
		case CONNECTED:

			UART8_SendString("WIFI ACTIVE\r\n");
			esp32_publish_heartbeat();
			esp32_listen_for_cloud_messages(5000);
			break;

		case DISCONNECTED:

			UART8_SendString("WIFI DISCONNECTED\r\n");
			if(esp32_join_ap((uint8_t*)SSID, (uint8_t *)PASSWORD) != ESP32_OK)
			{
				UART8_SendString("DISCONNECTED IN SUPERLOOP\r\n");
			}
			break;

		}

	}

}
