#include "esp_err.h"
#include "esp_log.h"
#include "app_config_mqtt.h"
#include "app_config.h"
#include "config_defs.h"
#include "app_config_mqtt_switch.h"

#define TAG "APP_CONFIG_MQTT_SWITCH"

void app_config_mqtt_switch_handler(char *topic, int topic_len, char *data, int data_len, void *user_data){
    app_config_mqtt_switch_t *sw = (app_config_mqtt_switch_t *)user_data;
    if(!strncmp(data, sw->payload_on, data_len)){
        app_config_mqtt_switch_set(1, sw);
    } else if(!strncmp(data, sw->payload_off, data_len)){
        app_config_mqtt_switch_set(0, sw);
    } else {
        ESP_LOGE(TAG, "Unrecognized command");
        return;
    }
    sw->cmd_handler(sw->state, sw);
    return;
}

app_config_mqtt_switch_t *app_config_mqtt_switch_create(char *prefix, char *obj_id, char *name, app_config_mqtt_switch_handler_t cmd_handler, bool discovery){
    ESP_LOGD(TAG, "Creating MQTT switch");
    app_config_mqtt_switch_t *sw = (app_config_mqtt_switch_t *)malloc(sizeof(app_config_mqtt_switch_t));
    sw->cmd_handler = cmd_handler;
    int state_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR) + 3;
    int cmd_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_CMD_STR) + 3;
    int avail_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR) + 3;
    sw->state_topic = (char *)malloc(state_topic_len);
    sw->cmd_topic = (char *)malloc(cmd_topic_len);
    sw->avail_topic = (char *)malloc(avail_topic_len);
    snprintf(sw->state_topic, state_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR);
    ESP_LOGD(TAG, "State topic: %s", sw->state_topic);
    snprintf(sw->cmd_topic, cmd_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_CMD_STR);
    ESP_LOGD(TAG, "Command topic: %s", sw->cmd_topic);
    snprintf(sw->avail_topic, avail_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR);
    ESP_LOGD(TAG, "Avail topic: %s", sw->avail_topic);
    sw->payload_on = CONFIG_APP_CONFIG_MQTT_SWITCH_ON_STR;
    sw->payload_off = CONFIG_APP_CONFIG_MQTT_SWITCH_OFF_STR;
    sw->state = 0;
    sw->qos = 0;
    sw->retain = 0;

    app_config_mqtt_subscribe(sw->cmd_topic, app_config_mqtt_switch_handler, sw);

    bool disc;
    app_config_getBool("std_mqtt_disc", &disc);
    if(discovery && disc){
        int disc_topic_len = strlen(std_mqtt_prefix) + strlen("/switch/") + strlen(obj_id) + strlen("/config") + 1;
        char *disc_topic = (char *)malloc(disc_topic_len);
        char *disc_data = (char *)malloc(200);
        snprintf(disc_topic, disc_topic_len, "%s/%s/%s/%s", std_mqtt_prefix, "switch", obj_id, "config");
        snprintf(disc_data, 200, "{\"name\":\"%s\",\"stat_t\":\"%s\",\"cmd_t\":\"%s\",\"avty_t\":\"%s\",\"pl_on\":\"%s\",\"pl_off\":\"%s\",\"stat_on\":\"%s\",\"stat_on\":\"%s\",\"qos\":0,\"ret\":false}", name, sw->state_topic, sw->cmd_topic, sw->avail_topic, sw->payload_on, sw->payload_off, sw->payload_on, sw->payload_off);
        ESP_LOGI(TAG, "Sending discovery:\n%s\nto %s", disc_data, disc_topic);
        app_config_mqtt_publish(disc_topic, disc_data, 0);
        free(disc_topic);
        free(disc_data);
    }
    return(sw);
}

esp_err_t app_config_mqtt_switch_delete(app_config_mqtt_switch_t *sw){
    if(sw){
        free(sw->state_topic);
        free(sw->cmd_topic);
        free(sw->avail_topic);
        free(sw);
        return(ESP_OK);
    } else {
        return(ESP_ERR_INVALID_ARG);
    }
}

void app_config_mqtt_switch_publish(app_config_mqtt_switch_t *sw){
    app_config_mqtt_publish(sw->state_topic, sw->state ? "ON" : "OFF", false);
}

void app_config_mqtt_switch_set(uint8_t state, app_config_mqtt_switch_t *sw){
    sw->state = state;
    app_config_mqtt_publish(sw->state_topic, sw->state ? "ON" : "OFF", false);
}