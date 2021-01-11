/*
 * app_config_mqtt.h
 *
 *  Created on: 10 дек. 2020 г.
 *      Author: ximen
 */

#ifndef APP_CONFIG_MQTT_H_
#define APP_CONFIG_MQTT_H_

#include "esp_err.h"
#include "mqtt_client.h"

typedef struct {
    char *topic;
    char *msg;
} app_config_mqtt_lwt_t;

/**
 * @brief      Starts MQTT subsystem
 *
 * Creates MQTT client and connects to MQTT broker, 
 * if mqtt_enable element is set to true and MQTT broker address string is specified.
 *
 * @param[in]   handler  MQTT event hadler function
 * 
 * @return
 *             - ESP_OK if MQTT started successfully
 *             - ESP_ERR_INVALID_ARG if MQTT event hadler not specified
 *             - ESP_ERR_NOT_FOUND if broker string is empty
 */
esp_err_t app_config_mqtt_init(esp_event_handler_t handler, app_config_mqtt_lwt_t *lwt);

void app_config_mqtt_publish(char *topic, char *value, bool retain);
#endif /* APP_CONFIG_MQTT_H_ */