/*
 * esp32_at.c
 *
 *  Created on: 16-Jun-2026
 *      Author: Akhilesh
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp32_at.h"





char at_cmd[MAX_AT_CMD_SIZE];
//static char rx_buffer [MAX_BUFFER_SIZE];


esp32_rx_state_t esp32_rx;

extern DMA_HandleTypeDef hdma_uart7_rx;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart8;



void UART8_SendString(char *str)
{
    HAL_UART_Transmit(&huart8, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}


esp32_status_t esp32_init(void)
{
	esp32_status_t ret = ESP32_OK;

	    // 0. Dummy command to wake up and flush the parser
	memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
	sprintf((char *)at_cmd, "AT%s", AT_CMD_TERMINATOR);
	send_at_cmd_and_wait(at_cmd, &esp32_rx, 5000);

	// 1. Disable Echo
	memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
	sprintf((char *)at_cmd, "ATE%d%s", ESP32_ECHO_OFF, AT_CMD_TERMINATOR);

	ret = send_at_cmd_and_wait(at_cmd, &esp32_rx, 5000);

	if(ret != ESP32_OK) return ret;

    // 3. Set Station Mode
    memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
    sprintf((char*)at_cmd, "AT+CWMODE=%d%s", ESP32_STATION_MODE, AT_CMD_TERMINATOR);

    ret = send_at_cmd_and_wait(at_cmd, &esp32_rx, 5000);

    return ret;
}


esp32_status_t esp32_join_ap(const uint8_t *ssid, const uint8_t *password) {

	esp32_status_t ret;
	ret = ESP32_OK;

	esp32_jap_error_t err;

	if((ssid == NULL) || (password == NULL)){ ret = ESP32_ERROR; return ret;}


	memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
	sprintf((char*)at_cmd, "AT+CWJAP=\"%s\",\"%s\"%s",ssid, password ,AT_CMD_TERMINATOR);

	ret = send_at_cmd_and_wait(at_cmd, &esp32_rx, 30000);

	if(ret == ESP32_ERROR)
	{
		err = esp32_jap_error((const char*)esp32_rx.buffer);

		if(err == CONN_TIMEOUT)
		{
			UART8_SendString("CONNECTION TIMEOUT\r\n");
		}
		else if(err == WRONG_PASSWORD)
		{
			UART8_SendString("WRONG PASSWORD\r\n");
		}
		else if(err == CANNOT_FIND_AP)
		{
			UART8_SendString("CANNOT FIND AP\r\n");
		}
		else if(err == CONN_FAIL)
		{
			UART8_SendString("CONNECTION FAILED");
		}
		else
		{
			UART8_SendString("OTHER REASONS\r\n");
		}

	}

	return ret;
}

esp32_jap_error_t esp32_jap_error(const char *buffer)
{
    if (buffer == NULL) {
        return OTHERS;
    }

    if (strstr(buffer, AT_AP_CONN_TIMEOUT) != NULL) {
        return CONN_TIMEOUT;
    } else if (strstr(buffer, AT_WRONG_PASS) != NULL) {
        return WRONG_PASSWORD;
    } else if (strstr(buffer, AT_CANT_FIND_AP) != NULL) {
        return CANNOT_FIND_AP;
    } else if (strstr(buffer, AT_CONN_FAIL) != NULL) {
        return CONN_FAIL;
    }else{
    return OTHERS;
    }
}



esp32_status_t esp32_config_sntp(int utc_offset)
{
	esp32_status_t ret;

	const char *ntp_server1 = "pool.ntp.org";
	const char *ntp_server2 = "time.google.com";

	memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
	sprintf((char*)at_cmd, "AT+CIPSNTPCFG=1,%d,\"%s\",\"%s\"%s", utc_offset,ntp_server1,ntp_server2, AT_CMD_TERMINATOR);

	ret = send_at_cmd_and_wait(at_cmd, &esp32_rx, 5000);

/*
	memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
	sprintf((char*)at_cmd, "AT+CIPSNTPTIME?%s", AT_CMD_TERMINATOR);

	ret = send_at_cmd_and_wait(at_cmd, &esp32_rx, 5000);
*/

	return ret;

}


esp32_status_t esp32_config_mqtt(void)
{
	esp32_status_t ret;

	memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
	sprintf((char*)at_cmd, "AT+MQTTUSERCFG=0,%d,\"%s\",\"\",\"\",%d,%d,\"\"%s",5,"my_iot_device_2",0,0,AT_CMD_TERMINATOR);

	ret = send_at_cmd_and_wait(at_cmd, &esp32_rx,5000);

	memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
	sprintf((char*)at_cmd, "AT+MQTTCONN=0,\"%s\",%u,%u%s","a1lx2t78z2215f-ats.iot.us-east-1.amazonaws.com",8883,1,AT_CMD_TERMINATOR);

	ret = send_at_cmd_and_wait(at_cmd, &esp32_rx,5000);

	return ret;

}


esp32_status_t esp32_publish_heartbeat(void)
{
    esp32_status_t ret;

    const char *payload = "{\"status\":\"alive\"}";

    memset(at_cmd, '\0', MAX_AT_CMD_SIZE);

    sprintf((char*)at_cmd, "AT+MQTTPUBRAW=0,\"device/heartbeat\",18,0,0%s", AT_CMD_TERMINATOR);

    ret = send_at_cmd_and_wait(at_cmd, &esp32_rx, 2000);

    if (ret != ESP32_OK) {
        return ret;
    }

    ret = send_at_cmd_and_wait(payload, &esp32_rx, 5000);

    return ret;
}



esp32_status_t esp32_mqtt_subscribe(const char *topic)
{
    esp32_status_t ret;

    memset(at_cmd, '\0', MAX_AT_CMD_SIZE);

    sprintf((char*)at_cmd, "AT+MQTTSUB=0,\"%s\",0%s", topic, AT_CMD_TERMINATOR);

    ret = send_at_cmd_and_wait(at_cmd, &esp32_rx, 2000);

    return ret;
}


void esp32_listen_for_cloud_messages(uint32_t listen_window_ms)
{
    // 1. Clear state and arm the DMA to listen
    HAL_UART_Abort(&huart7);
    memset((void *)esp32_rx.buffer, '\0', MAX_BUFFER_SIZE);
    esp32_rx.complete_flag = 0;

    // Start listening for incoming cloud data
    if(HAL_UARTEx_ReceiveToIdle_DMA(&huart7, (uint8_t *)esp32_rx.buffer, MAX_BUFFER_SIZE - 1) != HAL_OK) {
        return;
    }
    __HAL_DMA_DISABLE_IT(&hdma_uart7_rx, DMA_IT_HT);

    // 2. Wait for data OR timeout
    uint32_t start_tick = HAL_GetTick();
    while (!esp32_rx.complete_flag) {
        if ((HAL_GetTick() - start_tick) > listen_window_ms) {
            HAL_UART_Abort(&huart7); // Timeout reached, stop listening
            return;
        }
    }


    // 4. Parse the buffer for the ESP32's receive string
    char *mqtt_msg = strstr((const char *)esp32_rx.buffer, "+MQTTSUBRECV");

    if (mqtt_msg != NULL) {

        UART8_SendString("\r\n--- CLOUD DATA RECEIVED ---\r\n");
        UART8_SendString(mqtt_msg);
        UART8_SendString("\r\n---------------------------\r\n");
    }
}



/*


esp32_status_t send_at_cmd_and_wait(const char * const cmd,
                                    esp32_rx_state_t * const rx_state,
                                    const uint32_t timeout_ms)
{
	esp32_status_t ret;

    // 1. ALWAYS force the UART/DMA into a ready state before a new reception.
    // This ensures the DMA pointer resets back to buffer[0].
    HAL_UART_AbortReceive(&huart7);

    // 2. Reset the structure state
    memset((void *)rx_state->buffer, '\0', MAX_BUFFER_SIZE);
    rx_state->complete_flag = 0;
    rx_state->length = 0;

    // 3. Arm the DMA - Check for HAL_OK to ensure it actually started!
    if(HAL_UARTEx_ReceiveToIdle_DMA(&huart7, (uint8_t *)rx_state->buffer, MAX_BUFFER_SIZE - 1) != HAL_OK) {
        ret = ESP32_ERROR;
    }
    __HAL_DMA_DISABLE_IT(&hdma_uart7_rx, DMA_IT_HT);

    // 4. Send the AT Command
    if(HAL_UART_Transmit(&huart7, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY) != HAL_OK) {
        ret = ESP32_ERROR;
   }


    // 5. Wait for the response
    const uint32_t start_tick = HAL_GetTick();

    while (!rx_state->complete_flag) {
        if ((HAL_GetTick() - start_tick) > timeout_ms) {
            HAL_UART_AbortReceive(&huart7);
            ret = ESP32_TIMEOUT;
            return ret;
        }
    }

    // 6. Give the ESP32 parser a tiny breather (50ms) before the next command
    // This prevents the ESP32 from swallowing characters on back-to-back commands.
    HAL_Delay(50);



    // 7. Parse the linear buffer for expected strings
    if (strstr((const char *)rx_state->buffer, AT_OK_STRING) != NULL) {
        ret = ESP32_OK;
    } else if (strstr((const char *)rx_state->buffer, AT_ERROR_STRING) != NULL) {
        ret = ESP32_ERROR;
    }else if(strstr((const char*)rx_state->buffer, AT_MQTT_CONNECTED) != NULL){
    	ret = ESP32_OK;
    }else if(strstr((const char*)rx_state->buffer, AT_MQTT_DISCONNECTED) != NULL){
    	ret = ESP32_ERROR;
    }

    return ret;
}

*/

esp32_status_t send_at_cmd_and_wait(const char * const cmd,
                                    esp32_rx_state_t * const rx_state,
                                    const uint32_t timeout_ms)
{
    esp32_status_t ret = ESP32_TIMEOUT;

    // 1. Force UART/DMA into a ready state
    HAL_UART_AbortReceive(&huart7);

    // 2. Reset the state
    memset((void *)rx_state->buffer, '\0', MAX_BUFFER_SIZE);
    rx_state->complete_flag = 0;
    rx_state->length = 0;

    // 3. Arm the initial DMA transfer
    if(HAL_UARTEx_ReceiveToIdle_DMA(&huart7, (uint8_t *)rx_state->buffer, MAX_BUFFER_SIZE - 1) != HAL_OK) {
        return ESP32_ERROR;
    }
    __HAL_DMA_DISABLE_IT(&hdma_uart7_rx, DMA_IT_HT);

    // 4. Send the Command
    if(HAL_UART_Transmit(&huart7, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY) != HAL_OK) {
        return ESP32_ERROR;
    }

    // 5. Wait for the definitive string, accumulating chunks if necessary
    const uint32_t start_tick = HAL_GetTick();

    while ((HAL_GetTick() - start_tick) <= timeout_ms) {

        if (rx_state->complete_flag) {

            rx_state->complete_flag = 0; // Clear flag to ready up for the next chunk

            // Check if our accumulated buffer contains the final response
            if (strstr((const char *)rx_state->buffer, AT_OK_STRING) != NULL) {
                ret = ESP32_OK;
                break;
            } else if (strstr((const char *)rx_state->buffer, AT_ERROR_STRING) != NULL) {
                ret = ESP32_ERROR;
                break;
            }



            // The ESP32 paused, but we haven't found OK or ERROR. It sent a chunk.
            // We need to re-arm the DMA to append the next chunk to the end of our buffer.
            uint16_t remaining_space = MAX_BUFFER_SIZE - rx_state->length - 1;

            if (remaining_space > 0) {

                // Notice the offset: &rx_state->buffer[rx_state->length]
                if(HAL_UARTEx_ReceiveToIdle_DMA(&huart7, (uint8_t *)&rx_state->buffer[rx_state->length], remaining_space) != HAL_OK) {
                    ret = ESP32_ERROR;
                    break;
                }
                __HAL_DMA_DISABLE_IT(&hdma_uart7_rx, DMA_IT_HT);

            } else {
                // Buffer is completely full but no terminator found. Increase the buffer size if it comes here
                ret = ESP32_ERROR;
                break;
            }
        }
    }

    // 6. Stop any ongoing reception once we have our answer or timed out
    HAL_UART_AbortReceive(&huart7);

    return ret;
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == UART7) {
        // 'Size' is the number of bytes received in THIS specific DMA burst.
        // We add it to our running total.
        esp32_rx.length += Size;

        // Safety check to prevent buffer overflow
        if (esp32_rx.length >= MAX_BUFFER_SIZE) {
            esp32_rx.length = MAX_BUFFER_SIZE - 1;
        }

        // Null-terminate the string right at the end of the accumulated data
        esp32_rx.buffer[esp32_rx.length] = '\0';

        esp32_rx.complete_flag = 1;
    }
}

