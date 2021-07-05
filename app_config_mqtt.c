/*
 * app_config_mqtt.c
 *
 *  Created on: 10 дек. 2020 г.
 *      Author: ximen
 */

#include "esp_err.h"
#include "esp_log.h"
#include "app_config_mqtt.h"
#include "app_config.h"
#include "mqtt_client.h"

#define TAG "APP_CONFIG_MQTT"

static esp_mqtt_client_handle_t mqtt_client;
static app_config_mqtt_topic_sub_t topic_subscriptions[CONFIG_APP_CONFIG_MQTT_MAX_TOPIC_SUB];
static app_config_mqtt_event_sub_t event_subscriptions[] = {
                                                            {MQTT_EVENT_ANY, NULL},
                                                            {MQTT_EVENT_ERROR, NULL},
                                                            {MQTT_EVENT_CONNECTED, NULL},
                                                            {MQTT_EVENT_DISCONNECTED, NULL},
                                                            {MQTT_EVENT_SUBSCRIBED, NULL},
                                                            {MQTT_EVENT_UNSUBSCRIBED, NULL},
                                                            {MQTT_EVENT_PUBLISHED, NULL},
                                                            {MQTT_EVENT_DATA, NULL},
                                                        };

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGD(TAG, "MQTT_EVENT_DATA");
        for (uint8_t i = 0; i < SIZEOF(topic_subscriptions); i++){
            if(topic_subscriptions[i].topic && topic_subscriptions[i].handler){
                if(!strncmp(topic_subscriptions[i].topic, event->topic, event->topic_len)){
                    topic_subscriptions[i].handler(event->topic, event->topic_len, event->data, event->data_len, topic_subscriptions[i].user_data);
                }
            }
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGD(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGD(TAG, "Other event id:%d", event->event_id);
        break;
    }
    for (uint8_t i = 0; i < SIZEOF(event_subscriptions); i++){
        if((event_subscriptions[i].event == event_id) && event_subscriptions[i].handler){
            event_subscriptions[i].handler(event);
            break;
        }
    }
}

esp_err_t app_config_mqtt_init(app_config_mqtt_lwt_t *lwt){
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
            ESP_LOGD(TAG, "MQTT connect string %s", mqtt_uri);
            esp_mqtt_client_config_t mqtt_cfg = {
                .uri = mqtt_uri,
            };
            if(lwt){
                mqtt_cfg.lwt_topic = lwt->topic;
                mqtt_cfg.lwt_msg = lwt->msg;
                mqtt_cfg.lwt_retain = 1;
                mqtt_cfg.lwt_qos = 1;
            }
            mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
            esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_client);
            esp_mqtt_client_start(mqtt_client);
        } else {
            ESP_LOGE(TAG, "Error retrieving MQTT broker string");
            return ESP_ERR_NOT_FOUND;
        }
        return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

void app_config_mqtt_publish(char *topic, char *value, bool retain){
    esp_mqtt_client_publish(mqtt_client, topic, value, 0, 1, retain);
}

esp_err_t app_config_mqtt_subscribe(const char *topic, app_config_mqtt_handler_t handler, void *user_data){
    if(!handler) {
        ESP_LOGE(TAG, "Empty subscription handler!");
        return(ESP_ERR_INVALID_ARG);
    }
    ESP_LOGD(TAG, "Subscribing to %s", topic);
    for (uint8_t i = 0; i < SIZEOF(topic_subscriptions); i++){
        if(!topic_subscriptions[i].topic){
            topic_subscriptions[i].topic = topic;
            topic_subscriptions[i].handler = handler;
            topic_subscriptions[i].user_data = user_data;
            esp_mqtt_client_subscribe(mqtt_client, topic, 0);
            return(ESP_OK);
        }
    }
    return(ESP_ERR_NO_MEM);
}

esp_err_t app_config_mqtt_unsubscribe(char *topic, app_config_mqtt_handler_t handler){
    if(!topic) {
        ESP_LOGD(TAG, "Unsubscribing handler from all topics");
        for (uint8_t i = 0; i < SIZEOF(topic_subscriptions); i++){
            if(topic_subscriptions[i].handler == handler){
                topic_subscriptions[i].topic = NULL;
                topic_subscriptions[i].handler = NULL;
                topic_subscriptions[i].user_data = NULL;
            }
        }
        return(ESP_OK);
    }
    if(!handler) {
        ESP_LOGD(TAG, "Unsubscribing all handlers from topics %s", topic);
        for (uint8_t i = 0; i < SIZEOF(topic_subscriptions); i++){
            if(!strcmp(topic_subscriptions[i].topic, topic)){
                topic_subscriptions[i].topic = NULL;
                topic_subscriptions[i].handler = NULL;
                topic_subscriptions[i].user_data = NULL;
            }
        }
        return(ESP_OK);

    }
    for (uint8_t i = 0; i < SIZEOF(topic_subscriptions); i++){
        if(!strcmp(topic_subscriptions[i].topic, topic) && (topic_subscriptions[i].handler == handler)){
            topic_subscriptions[i].topic = NULL;
            topic_subscriptions[i].handler = NULL;
            topic_subscriptions[i].user_data = NULL;
            return(ESP_OK);
        }
    }
    return(ESP_ERR_NOT_FOUND);
}

esp_err_t app_config_mqtt_register_cb(esp_mqtt_event_id_t event, app_config_mqtt_event_handler_t handler){
    ESP_LOGD(TAG, "Registering callback to event %d", event);
    for (uint8_t i = 0; i < SIZEOF(event_subscriptions); i++){
        if(event_subscriptions[i].event == event){
            event_subscriptions[i].handler = handler;
            return(ESP_OK);
        }
    }
    return(ESP_ERR_NOT_FOUND);
}

esp_err_t app_config_mqtt_unregister_cb(esp_mqtt_event_id_t event){
    ESP_LOGD(TAG, "Unregistering callback from event %d", event);
    for (uint8_t i = 0; i < SIZEOF(event_subscriptions); i++){
        if(event_subscriptions[i].event == event){
            event_subscriptions[i].handler = NULL;
            return(ESP_OK);
        }
    }
    return(ESP_ERR_NOT_FOUND);
}
