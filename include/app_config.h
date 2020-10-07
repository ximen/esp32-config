/*
 * config.h
 *
 *  Created on: 18 мая 2020 г.
 *      Author: ximen
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include <stdbool.h>
#include "esp_err.h"
#include "sdkconfig.h"

#define CONF_NVS_NAMESPACE	CONFIG_APP_CONFIG_NVS_NAMESPACE

enum app_config_element_type_t{
	boolean,
	int8,
	int16,
	int32,
	//string,
	array
};

// Structure defines single configuration property 
typedef struct {
	char 							name[32];		// Configuration element's name
	char 							short_name[16];	// Configuration element's short name
	enum app_config_element_type_t	type;			// Configuration element's type
	size_t							size;			// Configuration element's length in bytes
	void							*value;
} app_config_element_t;

// Structure defines configuration topic collecting elements referring to one subject
typedef struct {
	char 				    name[32];			    // Configuration topic's name
	char 				    short_name[32];		    // Configuration topic's name
	uint8_t				    elements_number;	    // Number of elements in the topic
	app_config_element_t	*elements;			    // Pointer to array of configuration elements
} app_config_topic_t;

// Structure defines whole configuration
typedef struct {
	uint8_t				version;			        // Configuration structure version
	char 				name[32];			        // Configuration topic's name
	char 				short_name[32];		        // Configuration topic's name
	uint8_t				topics_number;		        // Number of elements in the topic
	app_config_topic_t	*topics;			        // Pointer to array of configuration elements
} app_config_t;


#define APP_CONFIG_DEFINE_BOOL(_short, _name)  { _name, #_short, boolean, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_INT8(_short, _name)  { _name, #_short, int8, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_INT16(_short, _name)  { _name, #_short, int16, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_INT32(_short, _name)  { _name, #_short, int32, sizeof(_short), &_short}
#define APP_CONFIG_DEFINE_ARRAY(_short, _name) { _name, #_short, array, sizeof(_short), _short}
#define APP_CONFIG_DEFINE_TOPIC(_short, _name, _elements) { _name, #_short, sizeof(_elements)/sizeof(_elements[0]), _elements}
#define APP_CONFIG_DEFINE_STD_WIFI(_name, _def_ssid, _def_psk) \
		static bool	std_wifi_ap = true; \
		static char	std_wifi_ssid[32] = _def_ssid; \
		static char	std_wifi_psk[64] = _def_psk; \
		static app_config_element_t config_std_wifi_elements[] = { \
		APP_CONFIG_DEFINE_BOOL(std_wifi_ap, "Access point"), \
		APP_CONFIG_DEFINE_ARRAY(std_wifi_ssid, "SSID"), \
		APP_CONFIG_DEFINE_ARRAY(std_wifi_psk, "PSK") };


/**
 * @brief      Initialize application configuration module
 *
 * Initialization takes place in three stages:
 * 1. Initialize NVS storage
 * 2. Open NVS storage
 * 3. Load stored configuration in global static variable app_conf
 *
  * @return
 *             - ESP_OK if configuration was initialized successfully
 *             - one of the error codes from the underlying flash storage driver
 */
esp_err_t app_config_init();

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
 */
esp_err_t app_config_getBool(const char* element, bool *value);

/**
 * @brief      Returns array from stored configuration
 *
 * Returns array of char stored in element 
 *
 * @param[in]   element  Name (label) of the required element
 * @param[out]  value    Pointer to char* variable for returning value
 *
 */
esp_err_t app_config_getArray(const char* element, char **value);

#endif /* APP_CONFIG_H_ */
