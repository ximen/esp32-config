/*
 * app_wifi.h
 *
 *  Created on: 19 мая 2020 г.
 *      Author: ximen
 */

#ifndef MAIN_APP_CONFIG_WIFI_H_
#define MAIN_APP_CONFIG_WIFI_H_

#include "esp_err.h"
#include "app_config.h"

#define APP_CONFIG_WIFI_MAXIMUM_RETRIES 4

typedef struct app_config_wifi_cb_arg app_config_wifi_cb_arg_t;
typedef struct app_config_ip_cb_arg app_config_ip_cb_arg_t;

typedef void(*app_config_wifi_cb_t)(wifi_event_t event, void *event_data);
typedef void(*app_config_ip_cb_t)(ip_event_t event, void *event_data);

struct app_config_wifi_cb_arg{
    wifi_event_t            event;
    app_config_wifi_cb_t    cb;
};

struct app_config_ip_cb_arg{
    ip_event_t              event;
    app_config_ip_cb_t      cb;
};

/**
 * @brief      Initialize and start WiFi with app_config configuration
 *
 * Loads configuration values from std_wifi element and starts WiFi with these values
 * Std_wifi element includes ap/sta flag, SSID and PSK values and IP configuration
 *
  * @return
 *             - ESP_OK if WiFi was initialized successfully
 *             - ESP_ERR_NOT_FOUND if std_wifi not defined in configuration
 *             - one of the error codes from in case of error
 */
esp_err_t app_config_wifi_init();

/**
 * @brief      Starts software access point
 *
 * Starts soft AP with predefined common values
  *
  * @return
 *             - ESP_OK if WiFi was initialized successfully
 *             - one of the error codes from in case of error
 */
esp_err_t app_config_ap_start();

/**
 * @brief      Starts WiFi in station mode
 *
 * Starts STA mode with predefined common values
  *
  * @return
 *             - ESP_OK if WiFi was initialized successfully
 *             - one of the error codes from in case of error
 */
esp_err_t app_config_sta_start();

esp_err_t app_config_wifi_register_cb(wifi_event_t event, app_config_wifi_cb_t cb);
esp_err_t app_config_ip_register_cb(ip_event_t event, app_config_ip_cb_t cb);

#endif /* MAIN_APP_CONFIG_WIFI_H_ */
