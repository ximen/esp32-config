/*
 * config.h
 *
 *  Created on: 18 мая 2020 г.
 *      Author: ximen
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "esp_err.h"
#include "sdkconfig.h"
#ifdef CONFIG_APP_CONFIG_BLUETOOTH_MESH
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_sensor_model_api.h"
#include "esp_ble_mesh_config_model_api.h"
#endif
#include "mqtt_client.h"
#include "app_config_mqtt.h"

#define CONF_NVS_NAMESPACE	CONFIG_APP_CONFIG_NVS_NAMESPACE
#define APP_CONFIG_MAX_SSID_LEN	32		// defined by WiFi standard
#define APP_CONFIG_MAX_PSK_LEN	63

enum app_config_element_type_t{
	boolean,
	int8,
	int16,
	int32,
	string,
	array,
	decimal
};

// Structure defines single configuration property 
typedef struct {
	char 							name[CONFIG_APP_CONFIG_ELT_NAME_LEN];			// Configuration element's name
	char 							short_name[CONFIG_APP_CONFIG_SHORT_NAME_LEN];	// Configuration element's short name
	enum app_config_element_type_t	type;											// Configuration element's type
	size_t							size;											// Configuration element's value length in bytes
	void							*value;											// Configuration element's value itself
} app_config_element_t;

// Structure defines configuration topic collecting elements referring to one subject
typedef struct {
	char 				    name[CONFIG_APP_CONFIG_ELT_NAME_LEN];			    // Configuration topic's name
	char 				    short_name[CONFIG_APP_CONFIG_SHORT_NAME_LEN];		// Configuration topic's name
	uint8_t				    elements_number;	    							// Number of elements in the topic
	app_config_element_t	*elements;			    							// Pointer to array of configuration elements
} app_config_topic_t;

// Structure defines whole configuration
typedef struct {
	uint8_t				version;			       			 					// Configuration structure version
	char 				name[CONFIG_APP_CONFIG_ELT_NAME_LEN];			        // Configuration topic's name
	char 				short_name[CONFIG_APP_CONFIG_SHORT_NAME_LEN];		    // Configuration topic's name
	uint8_t				topics_number;		        							// Number of elements in the topic
	app_config_topic_t	*topics;			        							// Pointer to array of configuration elements
} app_config_t;


#define APP_CONFIG_DEFINE_BOOL(_short, _name)  { _name, #_short, boolean, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_DECIMAL(_short, _name)  { _name, #_short, decimal, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_INT8(_short, _name)  { _name, #_short, int8, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_INT16(_short, _name)  { _name, #_short, int16, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_INT32(_short, _name)  { _name, #_short, int32, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_STRING(_short, _name)  { _name, #_short, string, sizeof(_short), _short}
#define APP_CONFIG_DEFINE_ARRAY(_short, _name) { _name, #_short, array, sizeof(_short), _short}
#define APP_CONFIG_DEFINE_TOPIC(_short, _name, _elements) { _name, #_short, sizeof(_elements)/sizeof(_elements[0]), _elements}
#define APP_CONFIG_DEFINE_STD_WIFI(_name, _def_ssid, _def_psk) \
		static bool	std_wifi_ap = true; \
		static char	std_wifi_ssid[APP_CONFIG_MAX_SSID_LEN] = _def_ssid; \
		static char	std_wifi_psk[APP_CONFIG_MAX_SSID_LEN] = _def_psk; \
		static app_config_element_t config_std_wifi_elements[] = { \
		APP_CONFIG_DEFINE_BOOL(std_wifi_ap, "Access point"), \
		APP_CONFIG_DEFINE_STRING(std_wifi_ssid, "SSID"), \
		APP_CONFIG_DEFINE_STRING(std_wifi_psk, "PSK") };
#define APP_CONFIG_DEFINE_STD_MQTT(_name, _def_port, _disc) \
		static char	std_mqtt_broker[CONFIG_APP_CONFIG_MQTT_BROKER_LEN]; \
		static uint16_t	std_mqtt_port = _def_port; \
		static char	std_mqtt_user[CONFIG_APP_CONFIG_MQTT_USER_LEN]; \
		static char	std_mqtt_pass[CONFIG_APP_CONFIG_MQTT_PASS_LEN]; \
		static char std_mqtt_prefix[CONFIG_APP_CONFIG_MQTT_PREFIX_LEN] = "homeassistant"; \
		static char std_mqtt_objid[CONFIG_APP_CONFIG_MQTT_OBJID_LEN] = "esp"; \
		static bool std_mqtt_disc = _disc; \
		static app_config_element_t config_std_mqtt_elements[] = { 	\
		APP_CONFIG_DEFINE_STRING(std_mqtt_broker, "Broker"), 		\
		APP_CONFIG_DEFINE_INT16(std_mqtt_port, "Port"), 			\
		APP_CONFIG_DEFINE_STRING(std_mqtt_user, "Username"), 		\
		APP_CONFIG_DEFINE_STRING(std_mqtt_pass, "Password"), 		\
		APP_CONFIG_DEFINE_STRING(std_mqtt_prefix, "Prefix"), 		\
		APP_CONFIG_DEFINE_STRING(std_mqtt_objid, "Object ID"), 		\
		APP_CONFIG_DEFINE_BOOL(std_mqtt_disc, "Discovery")};
#ifdef CONFIG_APP_CONFIG_OTA
#define APP_CONFIG_DEFINE_STD_OTA()									\
		static bool std_ota_enable = false;							\
		static char std_ota_url[CONFIG_APP_CONFIG_OTA_URL_LEN];		\
		static app_config_element_t config_std_ota_elements[] = {	\
		APP_CONFIG_DEFINE_BOOL(std_ota_enable, "Enable"),			\
		APP_CONFIG_DEFINE_STRING(std_ota_url, "URL")}; 		
#endif

// Structure defining callbacks
typedef struct {
#ifdef CONFIG_APP_CONFIG_BLUETOOTH_MESH	
	esp_ble_mesh_cfg_server_cb_t	 	config_srv;
    esp_ble_mesh_generic_server_cb_t 	generic_srv;
	esp_ble_mesh_sensor_client_cb_t		sensor_client;
#endif
} app_config_cbs_t;

/**
 * @brief      Initialize application configuration module
 *
 * Initialization takes place in three stages:
 * 1. Initialize NVS storage
 * 2. Open NVS storage
 * 3. Load stored configuration in global static variable app_conf
 * After initialization finished various subsystem being started according element's values
 * such as Bluetooth Mesh, MQTT etc.
 * 
 * @param[in]   cbs  pointer to structure containing callbacks
 * 
 * @return
 *             - ESP_OK if configuration was initialized successfully
 *             - one of the error codes from the underlying flash storage driver
 */
esp_err_t app_config_init(app_config_cbs_t *cbs);

/**
 * @brief      Returns pointer to global configuration structure
 *
 * Returns pointer to global application configuration structure
 * Should not be used outside app_config component
 * Access to configuration data should be done via app_config_get* funtions
 *
 * @return
 *             - app_config_t* pointer to configuration structure
 */
app_config_t *app_config_get();

/**
 * @brief      Returns boolean value from stored configuration
 *
 * Returns value of boolean element by its name
 *
 * @param[in]   element  Name (label) of the required element
 * @param[out]  value    Pointer to boolean variable for returning value
 *
 * @return
 *             - ESP_OK if configuration was initialized successfully
 *             - one of the error codes in other case
*/
esp_err_t app_config_getBool(const char* element, bool *value);

/**
 * @brief      Returns float value from stored configuration
 *
 * Returns value of float element by its name
 *
 * @param[in]   element  Name (label) of the required element
 * @param[out]  value    Pointer to float variable for returning value
 *
 * @return
 *             - ESP_OK if configuration was initialized successfully
 *             - one of the error codes in other case
*/
esp_err_t app_config_getFloat(const char* element, float *value);

/**
 * @brief      Returns array from stored configuration
 *
 * Returns array of char stored in element 
 *
 * @param[in]   element  Name (label) of the required element
 * @param[out]  value    Pointer to char* variable for returning value
 *
 * @return
 *             - ESP_OK if configuration was initialized successfully
 *             - one of the error codes in other case
*/
esp_err_t app_config_getArray(const char* element, char **value);

/**
 * @brief      Returns string from stored configuration
 *
 * Returns pointer to null-terminated string stored in element 
 *
 * @param[in]   element  Name (label) of the required element
 * @param[out]  value    Pointer to char* variable for returning value
 *
 * @return
 *             - ESP_OK if configuration was initialized successfully
 *             - one of the error codes in other case
*/
esp_err_t app_config_getString(const char* element, char **value);

/**
 * @brief      Returns value from stored configuration
 *
 * Returns stored value of given type in specified pointer
 *
 * @param[in]   element  Name (label) of the required element
 * @param[in]   type  	 Type of requested element element
 * @param[out]  value    Pointer to variable for returning value
 *
 * @return
 *             - ESP_OK in case of success
 *             - ESP_ERR_INVALID_ARG if soecified type does not match element's type
 * 			   - ESP_ERR_NOT_FOUND if requested element not found in configuration
*/
esp_err_t app_config_getValue(const char* element, enum app_config_element_type_t type, void *value);

/**
 * @brief      Returns element's size
 *
 * Returns size of configuration's element. Requested element must be string or blob type
 *
 * @param[in]   element  Name (label) of the required element
 * @param[out]  value    Pointer to variable for returning value
 *
 * @return
 *             - ESP_OK in case of success
 *             - ESP_ERR_INVALID_ARG if requested element not string or array
 * 			   - ESP_ERR_NOT_FOUND if requested element not found in configuration
*/
esp_err_t app_config_getSize(const char* element, size_t *value);

/**
 * @brief      Sets configuration element's value
 *
 * Sets value of specified configuration's element depending on its type
 *
 * @param[in]   element  Name (label) of the element
 * @param[out]  value    Pointer to a new value of the element
 *
 * @return
 *             - ESP_OK in case of success
 *             - ESP_ERR_INVALID_ARG if element has unknown type
 *             - ESP_ERR_NOT_FOUND if element with specified name not found in configuration
*/
esp_err_t app_config_setValue(const char* element, void *value);

/**
 * @brief      Saves configuration
 *
 * Saves current configuration to NVS
 *
  * @return
 *             - ESP_OK in case of success
 *             - one of error codes if fails
*/
esp_err_t app_config_save();
void app_config_restart();

/**
 * @brief      Resets configuration
 *
 * Resets Ble mesh provisioning information, erases configuration data and restart MCU
 *
*/
void app_config_erase();

/**
 * @brief      Returns JSON representation of configuration
 *
 * Returns pointer to  string containing JSON representation of current configuration.
 * Memory for string allocated dynamically so free() should be called to avoid memory leak.
 *
 * @return
 *             - Pointer to resulting string
 *             - NULL in case of error
 */
char *app_config_toJSON();

