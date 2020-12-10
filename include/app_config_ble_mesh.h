/*
 * app_config_ble_mesh.h
 *
 *  Created on: 17 ноября 2020 г.
 *      Author: ximen
 */

#ifndef APP_CONFIG_BLE_MESH_H_
#define APP_CONFIG_BLE_MESH_H_

#include "esp_err.h"
#include "esp_ble_mesh_generic_model_api.h"

// Structure defining ble mesh callbacks
typedef struct {
    esp_ble_mesh_generic_server_cb_t generic_srv;
} app_config_ble_mesh_cb_t;


/**
 * @brief      Initialize Bluetooth subsystem
 * 
 * Initialize bluetooth controller
 *
 * @return
 *             - ESP_OK if WiFi was initialized successfully
 *             - one of the error codes in case of error
 */
esp_err_t bluetooth_init();

/**
 * @brief      Initialize Bluetooth Mesh subsystem
 *
 * Starts BLE Mesh. Prior calling this function all necessary callbacks should be registred.
 *
 * @return
 *             - ESP_OK if WiFi was initialized successfully
 *             - one of the error codes in case of error
 */
esp_err_t app_config_ble_mesh_init(app_config_ble_mesh_cb_t *cbs);

#endif /* APP_CONFIG_BLE_MESH_H_ */