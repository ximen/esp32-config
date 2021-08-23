#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

typedef struct app_config_mqtt_switch app_config_mqtt_switch_t;

typedef void(*app_config_mqtt_switch_handler_t)(uint8_t state, app_config_mqtt_switch_t *sw);

struct app_config_mqtt_switch{
    char                                *state_topic;
    char                                *cmd_topic;
    char                                *avail_topic;
    char                                *payload_on;
    char                                *payload_off;
    uint8_t                             state;
    uint8_t                             qos;
    uint8_t                             retain;
    app_config_mqtt_switch_handler_t    cmd_handler;
    void                                *user_data;
};

app_config_mqtt_switch_t *app_config_mqtt_switch_create(char *prefix, char *obj_id, char *name, app_config_mqtt_switch_handler_t cmd_handler, bool discovery, bool retain, void *user_data);
void app_config_mqtt_switch_set(uint8_t state, app_config_mqtt_switch_t *sw);
esp_err_t app_config_mqtt_switch_delete(app_config_mqtt_switch_t *sw);
void app_config_mqtt_switch_publish(app_config_mqtt_switch_t *sw);