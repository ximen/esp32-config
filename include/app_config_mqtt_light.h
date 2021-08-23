#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

typedef struct {                // basic light states
    uint8_t     onoff;
    uint8_t     brightness;     // 255 = 100%
    // TODO: color and other stuff
} app_config_light_state_t;

typedef struct app_config_mqtt_light app_config_mqtt_light_t;

typedef void(*app_config_mqtt_light_handler_t)(app_config_light_state_t state, app_config_mqtt_light_t *light);

struct app_config_mqtt_light{
    char                                *state_topic;
    char                                *cmd_topic;
    char                                *brightness_cmd_topic;
    char                                *brightness_state_topic;
    int                                 brightness_scale;
    char                                *avail_topic;
    char                                *payload_on;
    char                                *payload_off;
    app_config_light_state_t            state;
    uint8_t                             qos;
    uint8_t                             retain;
    app_config_mqtt_light_handler_t     cmd_handler;
    app_config_mqtt_light_handler_t     brightness_cmd_handler;
    void                                *user_data;
    // TODO: Color modes, temp and other
};

app_config_mqtt_light_t *app_config_mqtt_light_create(char *prefix, char *obj_id, char *name, app_config_mqtt_light_handler_t cmd_handler, app_config_mqtt_light_handler_t brightness_cmd_handler, bool discovery, bool retain, void *user_data);
void app_config_mqtt_light_set(app_config_light_state_t state, app_config_mqtt_light_t *light);
void app_config_mqtt_light_set_onoff(uint8_t state, app_config_mqtt_light_t *light);
void app_config_mqtt_light_set_brightness(uint8_t brightness, app_config_mqtt_light_t *light);
esp_err_t app_config_mqtt_light_delete(app_config_mqtt_light_t *light);
void app_config_mqtt_light_publish(app_config_mqtt_light_t *light);