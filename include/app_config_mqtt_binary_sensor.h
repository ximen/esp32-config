#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    None,
    battery,
    battery_charging,
    cold,
    connectivity,
    door,
    garage_door,
    gas,
    heat,
    light,
    lock,
    moisture,
    motion,
    moving,
    occupancy,
    opening,
    plug,
    power,
    presence,
    problem,
    safety,
    smoke,
    sound,
    vibration,
    window
} app_config_mqtt_binary_class_t;

typedef struct {
    char                                *state_topic;
    char                                *avail_topic;
    char                                *payload_on;
    char                                *payload_off;
    uint8_t                             state;
    uint8_t                             qos;
    uint8_t                             retain;
    app_config_mqtt_binary_class_t      device_class;
    void                                *user_data;
} app_config_mqtt_binary_t;

app_config_mqtt_binary_t *app_config_mqtt_binary_create(char *prefix, char *obj_id, char *name, bool discovery, app_config_mqtt_binary_class_t device_class, bool retain, void *user_data);
void app_config_mqtt_binary_set(uint8_t state, app_config_mqtt_binary_t *sw);
esp_err_t app_config_mqtt_binary_delete(app_config_mqtt_binary_t *sw);
void app_config_mqtt_binary_publish(app_config_mqtt_binary_t *sw);