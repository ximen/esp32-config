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

#endif /* MAIN_APP_CONFIG_WIFI_H_ */
