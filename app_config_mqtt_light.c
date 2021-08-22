#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_err.h"
#include "esp_log.h"
#include "app_config_mqtt.h"
#include "app_config.h"
#include "config_defs.h"
#include "app_config_mqtt_light.h"

#define TAG "APP_CONFIG_MQTT_LIGHT"

static void app_config_mqtt_light_onoff_handler(char *topic, int topic_len, char *data, int data_len, void *user_data){
    app_config_mqtt_light_t *light = (app_config_mqtt_light_t *)user_data;
    if(!strncmp(data, light->payload_on, data_len)){
        app_config_mqtt_light_set_onoff(1, light);
    } else if(!strncmp(data, light->payload_off, data_len)){
        app_config_mqtt_light_set_onoff(0, light);
    } else {
        ESP_LOGE(TAG, "Unrecognized command");
        return;
    }
    light->cmd_handler(light->state, light);
    return;
}

static void app_config_mqtt_light_brightness_handler(char *topic, int topic_len, char *data, int data_len, void *user_data){
    app_config_mqtt_light_t *light = (app_config_mqtt_light_t *)user_data;

    char val_str[4] = {0};
    strncpy(val_str, data, (data_len > 3)?(3):(data_len));
    long val = strtol(val_str, NULL, 10);
    // TODO: Brightness scale
    if(val>100){
        ESP_LOGW(TAG, "Wrong brightness value %d", (int)val);
        val = 100;
    }
    app_config_mqtt_light_set_brightness(val, light);
    light->brightness_cmd_handler(light->state, light);
    return;
}

app_config_mqtt_light_t *app_config_mqtt_light_create(char *prefix, char *obj_id, char *name, app_config_mqtt_light_handler_t cmd_handler, app_config_mqtt_light_handler_t brightness_cmd_handler, bool discovery, bool retain, void *user_data){
    ESP_LOGD(TAG, "Creating MQTT light");
    app_config_mqtt_light_t *light = (app_config_mqtt_light_t *)malloc(sizeof(app_config_mqtt_light_t));
    light->retain = retain;
    light->user_data = user_data;
    light->cmd_handler = cmd_handler;
    light->brightness_cmd_handler = brightness_cmd_handler;
    int state_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR) + 3;
    ESP_LOGD(TAG, "State topic length: %d", state_topic_len);
    int brightness_state_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_LIGHT_BRIGHTNESS_STATE_STR) + 3;
    ESP_LOGD(TAG, "Brightness state topic length: %d", brightness_state_topic_len);
    int cmd_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_CMD_STR) + 3;
    ESP_LOGD(TAG, "Cmd topic length: %d", cmd_topic_len);
    int brightness_cmd_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_LIGHT_BRIGHTNESS_CMD_STR) + 3;
    ESP_LOGD(TAG, "Brightness cmd topic length: %d", brightness_cmd_topic_len);
    char *mqtt_objid;
    app_config_getString("std_mqtt_objid", &mqtt_objid);
    int avail_topic_len = strlen(std_mqtt_prefix) + strlen(mqtt_objid) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR) + 3;
    ESP_LOGD(TAG, "Avail topic length: %d", avail_topic_len);
    light->state_topic = (char *)malloc(state_topic_len);
    if(!light->state_topic){
        ESP_LOGE(TAG,"Error allocating %d for state topic!", state_topic_len);
        return NULL;
    }
    light->brightness_state_topic = (char *)malloc(brightness_state_topic_len);
    if(!light->brightness_state_topic){
        ESP_LOGE(TAG,"Error allocating %d for brightness state topic!", brightness_state_topic_len);
        free(light->state_topic);
        return NULL;
    }
    light->cmd_topic = (char *)malloc(cmd_topic_len);
    if(!light->cmd_topic){    
        free(light->state_topic);
        free(light->brightness_state_topic);
        ESP_LOGE(TAG,"Error allocating %d for cmd topic!", cmd_topic_len);
        return NULL;
    }
    light->brightness_cmd_topic = (char *)malloc(brightness_cmd_topic_len);
    if(!light->brightness_cmd_topic){    
        free(light->state_topic);
        free(light->brightness_state_topic);
        free(light->cmd_topic);
        ESP_LOGE(TAG,"Error allocating %d for brughtness cmd topic!", brightness_cmd_topic_len);
        return NULL;
    }
    light->avail_topic = (char *)malloc(avail_topic_len);
    if(!light->avail_topic){    
        free(light->state_topic);
        free(light->brightness_state_topic);
        free(light->cmd_topic);
        free(light->brightness_cmd_topic);
        ESP_LOGE(TAG,"Error allocating %d for avail topic!", avail_topic_len);
        return NULL;
    }
    snprintf(light->state_topic, state_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR);
    ESP_LOGD(TAG, "State topic: %s", light->state_topic);
    snprintf(light->brightness_state_topic, brightness_state_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_LIGHT_BRIGHTNESS_STATE_STR);
    ESP_LOGD(TAG, "Brightness_tate topic: %s", light->brightness_state_topic);
    snprintf(light->cmd_topic, cmd_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_CMD_STR);
    ESP_LOGD(TAG, "Command topic: %s", light->cmd_topic);
    snprintf(light->brightness_cmd_topic, brightness_cmd_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_LIGHT_BRIGHTNESS_CMD_STR);
    ESP_LOGD(TAG, "Brightness command topic: %s", light->brightness_cmd_topic);
    snprintf(light->avail_topic, avail_topic_len, "%s/%s%s", std_mqtt_prefix, mqtt_objid, CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR);
    ESP_LOGD(TAG, "Avail topic: %s", light->avail_topic);
    light->payload_on = CONFIG_APP_CONFIG_MQTT_SWITCH_ON_STR;
    light->payload_off = CONFIG_APP_CONFIG_MQTT_SWITCH_OFF_STR;
    light->state.onoff = 0;
    light->state.brightness = 0;
    light->qos = 1;

    app_config_mqtt_subscribe(light->cmd_topic, app_config_mqtt_light_onoff_handler, light);
    app_config_mqtt_subscribe(light->brightness_cmd_topic, app_config_mqtt_light_brightness_handler, light);

    bool disc;
    app_config_getBool("std_mqtt_disc", &disc);
    if(discovery && disc){
        int disc_topic_len = strlen(std_mqtt_prefix) + strlen("/light/") + strlen(obj_id) + strlen("/config") + 1;
        ESP_LOGD(TAG, "Discovery topic length %d", disc_topic_len);
        char *disc_topic = (char *)malloc(disc_topic_len);
        if(!disc_topic){
            ESP_LOGE(TAG, "Error allocation %d for disc topic", disc_topic_len);
            return light;
        }
        int disc_data_len = strlen("{\"name\":\"\",\"stat_t\":\"\",\"cmd_t\":\"\",\"bri_stat_t\":\"\",\"bri_cmd_t\":\"\",\"bri_scl\":,\"on_cmd_type\":\"brightness\",\"avty_t\":\"\",\"pl_on\":\"\",\"pl_off\":\"\",\"qos\":,\"ret\":}") + strlen(name) + strlen(light->state_topic) + strlen(light->cmd_topic) + strlen(light->brightness_state_topic) + strlen(light->brightness_cmd_topic) + strlen("100") + strlen(light->avail_topic) + strlen(light->payload_on) + strlen(light->payload_off) + (light->retain ? strlen("true") : strlen("false")) + 2;
        ESP_LOGD(TAG, "Discovery data length %d", disc_data_len);
        char *disc_data = (char *)malloc(disc_data_len);
        if(!disc_data){
            ESP_LOGE(TAG, "Error allocation %d for disc data", disc_data_len);
            free(disc_topic);
            return light;
        }
        snprintf(disc_topic, disc_topic_len, "%s/%s/%s/%s", std_mqtt_prefix, "light", obj_id, "config");
        snprintf(disc_data, 
                disc_data_len, 
                "{\"name\":\"%s\",\"stat_t\":\"%s\",\"cmd_t\":\"%s\",\"bri_stat_t\":\"%s\",\"bri_cmd_t\":\"%s\",\"bri_scl\":100,\"on_cmd_type\":\"brightness\",\"avty_t\":\"%s\",\"pl_on\":\"%s\",\"pl_off\":\"%s\",\"qos\":%d,\"ret\":%s}", 
                name, 
                light->state_topic, 
                light->cmd_topic, 
                light->brightness_state_topic, 
                light->brightness_cmd_topic, 
                light->avail_topic, 
                light->payload_on, 
                light->payload_off, 
                light->qos, 
                light->retain ? "true" : "false");
        ESP_LOGD(TAG, "Sending discovery:\n%s\nto %s", disc_data, disc_topic);
        app_config_mqtt_publish(disc_topic, disc_data, retain);
        free(disc_topic);
        free(disc_data);
    }
    return(light);
}

esp_err_t app_config_mqtt_light_delete(app_config_mqtt_light_t *light){
    if(light){
        free(light->state_topic);
        free(light->cmd_topic);
        free(light->avail_topic);
        free(light->brightness_state_topic);
        free(light->brightness_cmd_topic);        
        free(light);
        return(ESP_OK);
    } else {
        return(ESP_ERR_INVALID_ARG);
    }
}

void app_config_mqtt_light_publish(app_config_mqtt_light_t *light){
    ESP_LOGD(TAG, "Publishing state %d to %s", light->state.onoff, light->state_topic);
    app_config_mqtt_publish(light->state_topic, light->state.onoff ? light->payload_on : light->payload_off, light->retain);
    char br_str[5] = {0};
    snprintf(br_str, 4, "%d", light->state.brightness);
    ESP_LOGD(TAG, "Publishing brightness %s to %s", br_str, light->brightness_state_topic);
    app_config_mqtt_publish(light->brightness_state_topic, br_str, light->retain); // TODO: Brightness scale
}

void app_config_mqtt_light_set(app_config_light_state_t state, app_config_mqtt_light_t *light){
    light->state.onoff = state.onoff;
    light->state.brightness = state.brightness;
    app_config_mqtt_light_publish(light);
}

void app_config_mqtt_light_set_onoff(uint8_t state, app_config_mqtt_light_t *light){
    light->state.onoff = state;
    if(state) light->state.brightness = 100; else light->state.brightness = 0;
    app_config_mqtt_light_publish(light);
}

void app_config_mqtt_light_set_brightness(uint8_t brightness, app_config_mqtt_light_t *light){
    light->state.brightness = brightness;
    if(brightness > 0) light->state.onoff = 1; else light->state.onoff = 0;
    app_config_mqtt_light_publish(light);
}

void app_config_mqtt_light_set_avail_topic(char *topic, app_config_mqtt_light_t *light){
    free(light->avail_topic);
    light->avail_topic = (char *)malloc(strlen(topic) + 1);
    strncpy(light->avail_topic, topic, strlen(topic));
}

void app_config_mqtt_light_set_avail(uint8_t state, app_config_mqtt_light_t *light){
    app_config_mqtt_publish(light->avail_topic, state ? "online" : "offline", false);
}