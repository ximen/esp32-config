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

#define SIZEOF(arr)     sizeof(arr)/sizeof(arr[0])

typedef void(*app_config_mqtt_handler_t)(char *topic, int topic_len, char *data, int data_len, void *user_data);
typedef void(*app_config_mqtt_event_handler_t)(esp_mqtt_event_handle_t event);

typedef struct {
    const char                  *topic;
    app_config_mqtt_handler_t   handler;
    void                        *user_data;
} app_config_mqtt_topic_sub_t;

typedef struct {
    esp_mqtt_event_id_t                 event;
    app_config_mqtt_event_handler_t     handler;
} app_config_mqtt_event_sub_t;

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
 *             - ESP_ERR_NOT_SUPPORTED if MQTT is not enabled in config (mqtt_enable == false)
 */
esp_err_t app_config_mqtt_init(app_config_mqtt_lwt_t *lwt);

void app_config_mqtt_publish(char *topic, char *value, bool retain);

esp_err_t app_config_mqtt_subscribe(const char *topic, app_config_mqtt_handler_t handler, void *user_data);
esp_err_t app_config_mqtt_unsubscribe(char *topic, app_config_mqtt_handler_t handler);
esp_err_t app_config_mqtt_register_cb(esp_mqtt_event_id_t event, app_config_mqtt_event_handler_t handler);
esp_err_t app_config_mqtt_unregister_cb(esp_mqtt_event_id_t event);

#endif /* APP_CONFIG_MQTT_H_ */