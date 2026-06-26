#include"app_main.h"
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include<string.h>
#include "at25128b.h"

#include "esp32_at.h"
#include "esp32_at_io.h"
#include "xgzp6859.h"
#include <stdbool.h>

extern UART_HandleTypeDef huart7;
extern DMA_HandleTypeDef hdma_uart7_rx;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi3;

uint32_t pre = 99;
uint32_t backup[10];
uint8_t i;
uint8_t count;
bool wifi_status_flag = 1;

extern void MPU_Config(void);
extern void SystemClock_Config(void);


uint32_t address = 0x00;
uint8_t data_tx = 'C';
uint8_t data_rx;


uint32_t restored_pre;
uint8_t p[4];
uint8_t p1[4];
uint8_t page_select = 0;

int main()
{
	MPU_Config();

	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();

	MX_DMA_Init();
	MX_UART8_Init();
	MX_UART7_Init();
	MX_I2C1_Init();
	MX_SPI3_Init();

	if(at25128b_write_enable(&hspi3, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin) != HAL_OK)
	{
		Error_Handler();
	}

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

	if(MQTT_conn_state() != MQTT_CONNECTED_AND_SUBSCRIBED  )
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


	esp32_wifi_status_t status;
	status = esp32_cwstate();

	uint32_t start_tick = HAL_GetTick();





	while(1)
 	{

    	if(HAL_GetTick() - start_tick  >= 10)
    	{
    		status = esp32_cwstate();
    		pre = read_pressure();
    		start_tick = HAL_GetTick();
    	}



		switch(status)
		{
		case CONNECTED:

			if((wifi_status_flag == 0) && (count > 0) )
			{

				/*
				for(int j = 0; j < count; j++)
				{
					esp32_publish_pressure(&backup[j]);
					HAL_Delay(100);
				}
				count = 0;
				wifi_status_flag = 1;
				i = 0;
				*/

				if(page_select == 0)
				{
					address = 0x0000;
				}else if(page_select == 1)
				{
					address = 0x0040;
				}else
				{
					address = 0x0080;
				}

				while(count > 0)
				{

					if( at25128b_read_buffer(&hspi3, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, address, p1, 4) != HAL_OK)
					{
						Error_Handler();
					}

					restored_pre = ((uint32_t)p1[0] << 24) | ((uint32_t)p1[1] << 16) | ((uint32_t)p1[2] << 8) | (uint32_t)p1[3];

					esp32_publish_pressure(&restored_pre);

					address = address - 4;
					count--;
				}

				wifi_status_flag = 1;

			}

			UART8_SendString("WIFI ACTIVE\r\n");
			//esp32_publish_heartbeat();
			esp32_publish_pressure(&pre);
			esp32_listen_for_cloud_messages(5000);
			break;

		case DISCONNECTED:

			wifi_status_flag = false;


			UART8_SendString("WIFI DISCONNECTED\r\n");

			if(esp32_join_ap((uint8_t*)SSID, (uint8_t *)PASSWORD) != ESP32_OK)
			{
				UART8_SendString("DISCONNECTED IN SUPERLOOP\r\n");
			}

			p[0] = (pre >> 24) & 0xFF;
			p[1] = (pre >> 16) & 0xFF;
			p[2] = (pre >> 8)  & 0xFF;
			p[3] =  pre & 0xFF;

			if(at25128b_write_buffer(&hspi3, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, address, p, 4) != HAL_OK)
			{
				Error_Handler();
			}

			address = address + 4;
			count++;

			page_select++;
			if(page_select > 2){page_select = 0;}


/*			if(i<10)
			{
				backup[i] = pre;
				i++;

				if(count < 10)
				{
					count++;
				}

				if(i == 10)
				{
					i = 0;
				}
			}                                */
			break;

		}



	}

}






