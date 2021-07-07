//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_err.h"
#include "esp_log.h"
#include "app_config_mqtt.h"
#include "app_config.h"
#include "app_config_mqtt_binary_sensor.h"

#define TAG "APP_CONFIG_MQTT_BINARY"

typedef struct {
    app_config_mqtt_binary_class_t  num;
    char*                           string;
} device_class_strings_t;

static device_class_strings_t device_class_arr[] = {   
    {None, "None"},
    {battery, "battery"},
    {battery_charging, "battery_charging"},
    {cold, "cold"},
    {connectivity, "cold"},
    {door, "door"},
    {garage_door, "garage_door"},
    {gas, "gas",},
    {heat, "heat"},
    {light, "light"},
    {lock, "lock"},
    {moisture, "moisture"},
    {motion, "motion"},
    {moving, "moving"},
    {occupancy, "occupancy"},
    {opening, "opening"},
    {plug, "plug"},
    {power, "power"},
    {presence, "presence"},
    {problem, "problem"},
    {safety, "safety"},
    {smoke, "smoke"},
    {sound, "sound"},
    {vibration, "vibration"},
    {window, "window"}                          
};

char *get_device_class_str(app_config_mqtt_binary_class_t class){
    for (uint8_t i = 0; i < sizeof(device_class_arr) / sizeof(device_class_arr[0]); i++){
        if(device_class_arr[i].num == class) return device_class_arr[i].string;
    }
    return "None";
}

app_config_mqtt_binary_t *app_config_mqtt_binary_create(char *prefix, char *obj_id, char *name, bool discovery, app_config_mqtt_binary_class_t device_class, bool retain, void *user_data){
    ESP_LOGD(TAG, "Creating MQTT binary sensor");
    app_config_mqtt_binary_t *bs = (app_config_mqtt_binary_t *)malloc(sizeof(app_config_mqtt_binary_t));
    bs->user_data = user_data;
    bs->device_class = device_class;
    char *mqtt_prefix;
    app_config_getString("std_mqtt_prefix", &mqtt_prefix);
    int state_topic_len = strlen(prefix) + strlen(obj_id) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR) + 3;
    ESP_LOGD(TAG, "State topic length: %d", state_topic_len);
    char *mqtt_objid;
    app_config_getString("std_mqtt_objid", &mqtt_objid);
    int avail_topic_len = strlen(mqtt_prefix) + strlen(mqtt_objid) + strlen(CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR) + 3;
    ESP_LOGD(TAG, "Avail topic length: %d", avail_topic_len);
    bs->state_topic = (char *)malloc(state_topic_len);
    if(!bs->state_topic){
        ESP_LOGE(TAG,"Error allocating %d for state topic!", state_topic_len);
        return NULL;
    }
    bs->avail_topic = (char *)malloc(avail_topic_len);
    if(!bs->avail_topic){    
        free(bs->state_topic);
        ESP_LOGE(TAG,"Error allocating %d for avail topic!", avail_topic_len);
        return NULL;
    }
    snprintf(bs->state_topic, state_topic_len, "%s/%s%s", prefix, obj_id, CONFIG_APP_CONFIG_MQTT_SWITCH_STATE_STR);
    ESP_LOGD(TAG, "State topic: %s", bs->state_topic);
    snprintf(bs->avail_topic, avail_topic_len, "%s/%s%s", mqtt_prefix, mqtt_objid, CONFIG_APP_CONFIG_MQTT_SWITCH_AVAIL_STR);
    ESP_LOGD(TAG, "Avail topic: %s", bs->avail_topic);
    bs->payload_on = CONFIG_APP_CONFIG_MQTT_SWITCH_ON_STR;
    bs->payload_off = CONFIG_APP_CONFIG_MQTT_SWITCH_OFF_STR;
    bs->state = 0;
    bs->qos = 0;
    bs->retain = retain;

    bool disc;
    app_config_getBool("std_mqtt_disc", &disc);
    if(discovery && disc){
        int disc_topic_len = strlen(mqtt_prefix) + strlen("/binary_sensor/") + strlen(obj_id) + strlen("/config") + 1;
        ESP_LOGD(TAG, "Discovery topic length %d", disc_topic_len);
        char *disc_topic = (char *)malloc(disc_topic_len);
        if(!disc_topic){
            ESP_LOGE(TAG, "Error allocation %d for disc topic", disc_topic_len);
            return bs;
        }
        int disc_data_len = strlen("{\"name\":\"\",\"stat_t\":\"\",\"avty_t\":\"\",\"pl_on\":\"\",\"pl_off\":\"\",\"qos\":0,\"dev_cla\":\"\"}") + strlen(name) + strlen(bs->state_topic) + strlen(bs->avail_topic) + strlen(bs->payload_on) + strlen(bs->payload_off) + strlen(get_device_class_str(bs->device_class))+ 1;
        ESP_LOGD(TAG, "Discovery data length %d", disc_data_len);
        char *disc_data = (char *)malloc(disc_data_len);
        if(!disc_data){
            ESP_LOGE(TAG, "Error allocation %d for disc data", disc_data_len);
            free(disc_topic);
            return bs;
        }
        snprintf(disc_topic, disc_topic_len, "%s/%s/%s/%s", mqtt_prefix, "binary_sensor", obj_id, "config");
        snprintf(disc_data, disc_data_len, "{\"name\":\"%s\",\"stat_t\":\"%s\",\"avty_t\":\"%s\",\"pl_on\":\"%s\",\"pl_off\":\"%s\",\"qos\":0,\"dev_cla\":\"%s\"}", name, bs->state_topic, bs->avail_topic, bs->payload_on, bs->payload_off, get_device_class_str(bs->device_class));
        ESP_LOGD(TAG, "Sending discovery:\n%s\nto %s", disc_data, disc_topic);
        app_config_mqtt_publish(disc_topic, disc_data, bs->retain);
        free(disc_topic);
        free(disc_data);
    }
    return(bs);
}

void app_config_mqtt_binary_set(uint8_t state, app_config_mqtt_binary_t *bs){
    bs->state = state;
    app_config_mqtt_binary_publish(bs);

}

esp_err_t app_config_mqtt_binary_delete(app_config_mqtt_binary_t *bs){
    if(bs){
        free(bs->state_topic);
        free(bs->avail_topic);
        free(bs);
        return(ESP_OK);
    } else {
        return(ESP_ERR_INVALID_ARG);
    }
}

void app_config_mqtt_binary_publish(app_config_mqtt_binary_t *bs){
    ESP_LOGD(TAG, "Publishing state %d to %s", bs->state, bs->state_topic);
    app_config_mqtt_publish(bs->state_topic, bs->state ? bs->payload_on : bs->payload_off, bs->retain);
}