//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
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

app_config_mqtt_switch_t *app_config_mqtt_switch_create(char *prefix, char *obj_id, char *name, app_config_mqtt_switch_handler_t cmd_handler, bool discovery, bool retain, void *user_data){
    ESP_LOGD(TAG, "Creating MQTT switch");
    app_config_mqtt_switch_t *sw = (app_config_mqtt_switch_t *)malloc(sizeof(app_config_mqtt_switch_t));
    sw->retain = retain;
    sw->user_data = user_data;
    sw->cmd_handler = cmd_handler;
    int state_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR) + 3;
    ESP_LOGD(TAG, "State topic length: %d", state_topic_len);
    int cmd_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_CMD_STR) + 3;
    ESP_LOGD(TAG, "Cmd topic length: %d", cmd_topic_len);
    char *mqtt_objid;
    app_config_getString("std_mqtt_objid", &mqtt_objid);
    int avail_topic_len = strlen(std_mqtt_prefix) + strlen(mqtt_objid) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR) + 3;
    ESP_LOGD(TAG, "Avail topic length: %d", avail_topic_len);
    sw->state_topic = (char *)malloc(state_topic_len);
    if(!sw->state_topic){
        ESP_LOGE(TAG,"Error allocating %d for state topic!", state_topic_len);
        return NULL;
    }
    sw->cmd_topic = (char *)malloc(cmd_topic_len);
    if(!sw->cmd_topic){    
        free(sw->state_topic);
        ESP_LOGE(TAG,"Error allocating %d for cmd topic!", cmd_topic_len);
        return NULL;
    }
    sw->avail_topic = (char *)malloc(avail_topic_len);
    if(!sw->avail_topic){    
        free(sw->state_topic);
        free(sw->cmd_topic);
        ESP_LOGE(TAG,"Error allocating %d for avail topic!", avail_topic_len);
        return NULL;
    }
    snprintf(sw->state_topic, state_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR);
    ESP_LOGD(TAG, "State topic: %s", sw->state_topic);
    snprintf(sw->cmd_topic, cmd_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_CMD_STR);
    ESP_LOGD(TAG, "Command topic: %s", sw->cmd_topic);
    snprintf(sw->avail_topic, avail_topic_len, "%s/%s%s", std_mqtt_prefix, mqtt_objid, CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR);
    ESP_LOGD(TAG, "Avail topic: %s", sw->avail_topic);
    sw->payload_on = CONFIG_APP_CONFIG_MQTT_SWITCH_ON_STR;
    sw->payload_off = CONFIG_APP_CONFIG_MQTT_SWITCH_OFF_STR;
    sw->state = 0;
    sw->qos = 1;

    app_config_mqtt_subscribe(sw->cmd_topic, app_config_mqtt_switch_handler, sw);

    bool disc;
    app_config_getBool("std_mqtt_disc", &disc);
    if(discovery && disc){
        int disc_topic_len = strlen(std_mqtt_prefix) + strlen("/switch/") + strlen(obj_id) + strlen("/config") + 1;
        ESP_LOGD(TAG, "Discovery topic length %d", disc_topic_len);
        char *disc_topic = (char *)malloc(disc_topic_len);
        if(!disc_topic){
            ESP_LOGE(TAG, "Error allocation %d for disc topic", disc_topic_len);
            return sw;
        }
        int disc_data_len = strlen("{\"name\":\"\",\"stat_t\":\"\",\"cmd_t\":\"\",\"avty_t\":\"\",\"pl_on\":\"\",\"pl_off\":\"\",\"stat_on\":\"\",\"stat_off\":\"\",\"qos\":,\"ret\":}") + strlen(name) + strlen(sw->state_topic) + strlen(sw->cmd_topic) + strlen(sw->avail_topic) + strlen(sw->payload_on) + strlen(sw->payload_off) + strlen(sw->payload_on) + strlen(sw->payload_off) + (sw->retain ? strlen("true") : strlen("false")) + 2;
        ESP_LOGD(TAG, "Discovery data length %d", disc_data_len);
        char *disc_data = (char *)malloc(disc_data_len);
        if(!disc_data){
            ESP_LOGE(TAG, "Error allocation %d for disc data", disc_data_len);
            free(disc_topic);
            return sw;
        }
        snprintf(disc_topic, disc_topic_len, "%s/%s/%s/%s", std_mqtt_prefix, "switch", obj_id, "config");
        snprintf(disc_data, disc_data_len, "{\"name\":\"%s\",\"stat_t\":\"%s\",\"cmd_t\":\"%s\",\"avty_t\":\"%s\",\"pl_on\":\"%s\",\"pl_off\":\"%s\",\"stat_on\":\"%s\",\"stat_off\":\"%s\",\"qos\":%d,\"ret\":%s}", name, sw->state_topic, sw->cmd_topic, sw->avail_topic, sw->payload_on, sw->payload_off, sw->payload_on, sw->payload_off, sw->qos, sw->retain ? "true" : "false");
        ESP_LOGD(TAG, "Sending discovery:\n%s\nto %s", disc_data, disc_topic);
        app_config_mqtt_publish(disc_topic, disc_data, retain);
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
    ESP_LOGD(TAG, "Publishing state %d to %s", sw->state, sw->state_topic);
    app_config_mqtt_publish(sw->state_topic, sw->state ? sw->payload_on : sw->payload_off, sw->retain);
}

void app_config_mqtt_switch_set(uint8_t state, app_config_mqtt_switch_t *sw){
    sw->state = state;
    app_config_mqtt_switch_publish(sw);
}

void app_config_mqtt_switch_set_avail_topic(char *topic, app_config_mqtt_switch_t *sw){
    free(sw->avail_topic);
    sw->avail_topic = (char *)malloc(strlen(topic) + 1);
    strncpy(sw->avail_topic, topic, strlen(topic));
}

void app_config_mqtt_switch_set_avail(uint8_t state, app_config_mqtt_switch_t *sw){
    app_config_mqtt_publish(sw->avail_topic, state ? "online" : "offline", false);
}