/*
 * app_config_mqtt.c
 *
 *  Created on: 10 дек. 2020 г.
 *      Author: ximen
 */

#include "esp_err.h"
#include "app_config_mqtt.h"
#include "app_config.h"
#include "mqtt_client.h"

#define TAG "APP_CONFIG_MQTT"

static esp_mqtt_client_handle_t mqtt_client;

esp_err_t app_config_mqtt_init(esp_event_handler_t handler){
    if (!handler){
        ESP_LOGE(TAG, "MQTT handler is NULL. Aborting MQTT");
        return ESP_ERR_INVALID_ARG;

    }
    bool config_mqtt_enable;
    app_config_getBool("mqtt_enable", &config_mqtt_enable);
    if(config_mqtt_enable){
        ESP_LOGI(TAG, "MQTT enabled");
        char *mqtt_broker;
        char mqtt_uri[CONFIG_APP_CONFIG_MQTT_BROKER_LEN + 14];
        esp_err_t err = app_config_getValue("std_mqtt_broker", string, &mqtt_broker);
        if ((err == ESP_OK)&&(strlen(mqtt_broker) > 0)){
            int16_t port;
            err = app_config_getValue("std_mqtt_port", int16, &port);
            if (err != ESP_OK) port = 1883;
            sprintf(mqtt_uri, "mqtt://%s:%d", mqtt_broker, (uint16_t)port);
            ESP_LOGI(TAG, "MQTT connect string %s", mqtt_uri);
            esp_mqtt_client_config_t mqtt_cfg = {
                .uri = mqtt_uri,
            };
            mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
            esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, handler, mqtt_client);
            esp_mqtt_client_start(mqtt_client);
        } else {
            ESP_LOGE(TAG, "Error retrieving MQTT broker string");
            return ESP_ERR_NOT_FOUND;
        }
    }
    return ESP_OK;
}

void app_config_mqtt_publish(char *topic, char *value){
    esp_mqtt_client_publish(mqtt_client, topic, value, 0, 1, 0);
}