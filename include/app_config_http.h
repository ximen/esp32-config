/*
 * app_http.h
 *
 *  Created on: 19 сент. 2020 г.
 *      Author: ximen
 */

#ifndef MAIN_APP_CONFIG_HTTP_H_
#define MAIN_APP_CONFIG_HTTP_H_

#include "esp_err.h"
#include "app_config.h"

/**
 * @brief      Initialize HTTP server
 *
 * Starts HTTP server with genrated  configuration page
  *
  * @return
 *             - ESP_OK if WiFi was initialized successfully
 *             - one of the error codes from in case of error
 */
esp_err_t app_config_http_init();

#endif /* MAIN_APP_CONFIG_HTTP_H_ */
