/*
 * esp32_at.h
 *
 *  Created on: 16-Jun-2026
 *      Author: Akhilesh
 */

#ifndef COMPONENTS_INC_ESP32_AT_H_
#define COMPONENTS_INC_ESP32_AT_H_




#include <stdint.h>
#include "main.h"

#define AT_AP_CONN_TIMEOUT "+CWJAP:1"
#define AT_WRONG_PASS      "+CWJAP:2"
#define AT_CANT_FIND_AP    "+CWJAP:3"
#define AT_CONN_FAIL       "+CWJAP:4"

#define AP_CONNECTED       "+CWSTATE:2"

#define MAX_AT_CMD_SIZE         256
#define MAX_BUFFER_SIZE         (1024*3)
#define AT_CMD_TERMINATOR       "\r\n"
#define AT_OK_STRING            "OK\r\n"
#define AT_CONNECT_STRING       "CONNECT\r\n"
#define AT_IPD_OK_STRING        "OK\r\n\r\n"
#define AT_SEND_OK_STRING       "SEND OK\r\n"
#define AT_SEND_PROMPT_STRING   "OK\r\n\r\n>"
#define AT_ERROR_STRING         "ERROR\r\n"
#define AT_IPD_STRING           "+IPD,"
#define AT_HTTPCGET_STRING      "+HTTPCGET:"

#define AT_MQTT_DISCONNECTED       "+MQTTDISCONNECTED"
#define AT_MQTT_CONNECTED       "+MQTTCONNECTED"


#define SSID "Akhil"
#define PASSWORD "12345678"

#define UTC_OFFSET 5


typedef enum {
	ESP32_ECHO_OFF = 0,
	ESP32_ECHO_ON = 1
} esp32_echo_state_t;


typedef enum
{
	ESP32_NULL_MODE = 0,
	ESP32_STATION_MODE = 1,
	ESP32_SOFT_AP_MODE = 2,
	ESP32_SOFT_AP_STATION_MODE = 3
}esp32_mode_t;


typedef enum {
  ESP32_OK                      = 0,
  ESP32_ERROR                   = 1,
  ESP32_BUSY                    = 2,
  ESP32_ALREADY_CONNECTED       = 3,
  ESP32_CONNECTION_CLOSED       = 4,
  ESP32_TIMEOUT                 = 5,
  ESP32_IO_ERROR                = 6,
} esp32_status_t;




typedef struct {
    uint8_t buffer[MAX_BUFFER_SIZE];
    volatile uint16_t length;
    volatile uint8_t complete_flag;
} esp32_rx_state_t;


typedef enum
{
	CONN_TIMEOUT   = 0,
	WRONG_PASSWORD = 1,
	CANNOT_FIND_AP = 2,
	CONN_FAIL      = 3,
	OTHERS         = 4

}esp32_jap_error_t;


typedef enum
{
	CONNECTED = 0,
	DISCONNECTED = 1

}esp32_wifi_status_t;

typedef enum
{
	MQTT_UNINITIALIZED 		= 0,
	MQTT_ALREADYSET_USERCFG = 1,
	MQTT_ALREADYSET_CONNCFG = 2,
	MQTT_CONN_DISCONNECTED  = 3,
	MQTT_CONN_ESTABLISHED   = 4,
	MQTT_CONNECTED_BUT_NOT_SUBSCRIBED = 5,
	MQTT_CONNECTED_AND_SUBSCRIBED = 6,
	MQTT_OTHERS
}esp32_mqtt_status_t;



esp32_status_t esp32_init(void);

esp32_status_t send_at_cmd_and_wait(const char * const cmd,
                                    esp32_rx_state_t * const rx_state,
                                    const uint32_t timeout_ms);

esp32_status_t send_at_cmd(char *str);
esp32_status_t esp32_config_sntp(int utc_offset);

esp32_status_t esp32_join_ap(const uint8_t * ssid, const uint8_t * password);
esp32_jap_error_t esp32_jap_error(const char *buffer);


esp32_status_t esp32_config_mqtt(void);

esp32_status_t esp32_publish_heartbeat(void);
esp32_status_t esp32_mqtt_subscribe(const char *topic);

void esp32_listen_for_cloud_messages(uint32_t listen_window_ms);

esp32_wifi_status_t esp32_cwstate(void);
esp32_mqtt_status_t MQTT_conn_state(void);

void UART8_SendString(char *str);

#endif /* COMPONENTS_INC_ESP32_AT_H_ */


