/*
 * config.c
 *
 *  Created on: 18 мая 2020 г.
 *      Author: ximen
 */
#include "app_config.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "string.h"
#include "config_defs.h"
#include "cJSON.h"
#include "app_config_wifi.h"
#include "app_config_http.h"
#ifdef CONFIG_APP_CONFIG_BLUETOOTH_MESH
#include "app_config_ble_mesh.h"
#include "esp_ble_mesh_networking_api.h"
#endif
#include "app_config_mqtt.h"
#ifdef CONFIG_APP_CONFIG_OTA
#include "app_config_ota.h"
#endif

#define TAG "APP_CONFIG"
static nvs_handle_t app_config_nvs_hanle;
static app_config_t app_conf;

// Returns configuration singleton
app_config_t *app_config_get(){
	return &app_conf;
}

esp_err_t app_config_load_element(app_config_element_t *element){
	ESP_LOGD(TAG, "Loading element %s with size %d", element->short_name, element->size);
	esp_err_t err = ESP_OK;
	size_t tmp_size;
	switch(element->type){
	case boolean:		// Boolean storead as uint8_t
		err = nvs_get_u8(app_config_nvs_hanle, element->short_name, element->value);
		ESP_LOGD(TAG, "Got %d", *(uint8_t *)element->value);
		break;
	case array:
		tmp_size = element->size;
		err = nvs_get_blob(app_config_nvs_hanle, element->short_name, element->value, &tmp_size);
		ESP_LOGD(TAG, "Got %s", (char *)element->value);
		break;
	case string:
		tmp_size = element->size;
		err = nvs_get_str(app_config_nvs_hanle, element->short_name, element->value, &tmp_size);
		ESP_LOGI(TAG, "Got %s", (char *)element->value);
		break;
	case int8:
		err = nvs_get_u8(app_config_nvs_hanle, element->short_name, element->value);
		ESP_LOGD(TAG, "Got %d", *(uint8_t *)element->value);
		break;
	case int16:
		err = nvs_get_u16(app_config_nvs_hanle, element->short_name, element->value);
		ESP_LOGD(TAG, "Got %d", *(uint16_t *)element->value);
		break;
	case int32:
		err = nvs_get_u32(app_config_nvs_hanle, element->short_name, element->value);
		ESP_LOGD(TAG, "Got %d", *(uint32_t *)element->value);
		break;
	case decimal:;
		size_t len = sizeof(float);
		err = nvs_get_blob(app_config_nvs_hanle, element->short_name, element->value, &len);
		ESP_LOGD(TAG, "Got %f", *(float *)element->value);
		break;
	default:
		break;
	}
	return err;
}

esp_err_t app_config_load_topic(app_config_topic_t *topic){
	ESP_LOGD(TAG, "Loading topic %s with %d elements", topic->short_name, topic->elements_number);
	for (uint8_t i = 0; i < topic->elements_number; i++){
		esp_err_t err = app_config_load_element(&topic->elements[i]);
		if(err) {
			if (err == ESP_ERR_NVS_NOT_FOUND){
				ESP_LOGW(TAG, "Not found element %s", topic->elements[i].short_name);
			} else {
				ESP_LOGE(TAG, "Error while loading element %s (error %d). Aborting.", topic->elements[i].short_name, err);
				return err;
			}
		}
	}
	return ESP_OK;
}

esp_err_t app_config_load(){
	esp_err_t err;
	ESP_LOGI(TAG, "Checking saved configuration version");
	uint8_t version;
	err = nvs_get_u8(app_config_nvs_hanle, "config_version", &version);
	if (err) return err;
	ESP_LOGI(TAG, "Got version %d", version);
	if (version != app_conf.version) return ESP_ERR_INVALID_VERSION;
	ESP_LOGI(TAG, "Versions matched");
	ESP_LOGI(TAG, "Loading configuration with %d topics", app_conf.topics_number);
	for (uint8_t i = 0; i < app_conf.topics_number; i++){
		err = app_config_load_topic(&app_conf.topics[i]);
		if (err) return err;
	}
	return ESP_OK;
}

esp_err_t app_config_save_element(app_config_element_t *element){
	ESP_LOGD(TAG, "Saving element %s with size %d", element->short_name, element->size);
	esp_err_t err = ESP_OK;
	switch(element->type){
	case boolean:		// Boolean storead as uint8_t
		err = nvs_set_u8(app_config_nvs_hanle, element->short_name, *(uint8_t *)element->value);
		break;
	case array:
		err = nvs_set_blob(app_config_nvs_hanle, element->short_name, element->value, element->size);
		break;
	case string:
		ESP_LOGD(TAG,"Saving string %s, value %s", element->short_name, (char*)element->value);
		err = nvs_set_str(app_config_nvs_hanle, element->short_name, element->value);
		break;
	case int8:
		err = nvs_set_u8(app_config_nvs_hanle, element->short_name, *(uint8_t *)element->value);
		break;
	case int16:
		err = nvs_set_u16(app_config_nvs_hanle, element->short_name, *(uint16_t *)element->value);
		break;
	case int32:
		err = nvs_set_u32(app_config_nvs_hanle, element->short_name, *(uint32_t *)element->value);
		break;
	case decimal:
		err = nvs_set_blob(app_config_nvs_hanle, element->short_name, (void *)element->value, sizeof(float));
		break;
	default:
		break;
	}
	return err;
}

esp_err_t app_config_save_topic(app_config_topic_t *topic){
	for (uint8_t i = 0; i < topic->elements_number; i++){
		esp_err_t err = app_config_save_element(&topic->elements[i]);
		if (err){
			ESP_LOGE(TAG, "Error while saving element %s (error %d). Aborting.", topic->elements[i].short_name, err);
			return err;
		}
	}
	return ESP_OK;
}

esp_err_t app_config_save(){
	esp_err_t err;
	for (uint8_t i = 0; i < app_conf.topics_number; i++){
		err = app_config_save_topic(&app_conf.topics[i]);
		if (err){
			return err;
		}
	}
	err = nvs_set_u8(app_config_nvs_hanle, "config_version", app_conf.version);
	nvs_commit(app_config_nvs_hanle);
	return ESP_OK;
}

app_config_element_t *findElement(const char* element){
	for (uint8_t i=0; i < app_conf.topics_number; i++){
		for (uint8_t j=0; j < app_conf.topics[i].elements_number; j++){
			if (strncmp(element, app_conf.topics[i].elements[j].short_name, strlen(element) + 1) == 0){
				return &app_conf.topics[i].elements[j];
			}
		}
	}
	return NULL;
}

esp_err_t app_config_getValue(const char* element, enum app_config_element_type_t type, void *value){
	ESP_LOGD(TAG, "Getting from config: %s", element);
	app_config_element_t *elt = findElement(element);
	if(elt != NULL){
		if(elt->type != type) return ESP_ERR_INVALID_ARG;
		if(type == boolean) *(bool *)value = *(bool *)elt->value;
		if(type == int8) *(int8_t *)value = *(int8_t *)elt->value;		
		if(type == int16) *(int16_t *)value = *(int16_t *)elt->value;		
		if(type == int32) *(int32_t *)value = *(int32_t *)elt->value;		
		if(type == array) *(char **)value = (char *)elt->value;
		if(type == string) *(char **)value = (char *)elt->value;
		if(type == decimal) *(float *)value = *(float *)elt->value;
		return ESP_OK;
	} else {
		return ESP_ERR_NOT_FOUND;
	}
}

esp_err_t app_config_getSize(const char* element, size_t *value){
	ESP_LOGD(TAG, "Getting size of %s", element);
	app_config_element_t *elt = findElement(element);
	if(elt != NULL){
		if((elt->type == string)||(elt->type == array)){
			*value = elt->size;
		} else {
			ESP_LOGD(TAG, "Not string nor array");
			return ESP_ERR_INVALID_ARG;
		}
	} else {
		ESP_LOGD(TAG, "Element not found");
		return ESP_ERR_NOT_FOUND;
	}
	return ESP_OK;
}

esp_err_t app_config_setValue(const char* element, void *value){
	ESP_LOGD(TAG, "Setting element: %s", element);
	app_config_element_t *elt = findElement(element);
	if(elt != NULL){
		switch (elt->type){
		case boolean:
			*(bool *)elt->value = *(bool *)value;
			break;
		case int8:
			*(int8_t *)elt->value = *(int8_t *)value;
			break;		
		case int16:
			*(int16_t *)elt->value = *(int16_t *)value;
			break;		
		case int32:
			*(int32_t *)elt->value = *(int32_t *)value;
			break;		
		case array:
			strncpy((char *)elt->value, (char *)value, elt->size);
			break;		
		case string:
			strncpy((char *)elt->value, (char *)value, elt->size);
			ESP_LOGD(TAG, "Setting %s, value %s, size %d", elt->short_name, (char *)elt->value, elt->size);
			break;	
		case decimal:
			*(float *)elt->value = *(float *)value;	
			break;
		default:
			ESP_LOGD(TAG, "Wrong type");
			return ESP_ERR_INVALID_ARG;
		}
	} else {
		ESP_LOGD(TAG, "Element not found");
		return ESP_ERR_NOT_FOUND;
	}
	return ESP_OK;
}

esp_err_t app_config_getBool(const char* element, bool *value){
	return app_config_getValue(element, boolean, value);
}

esp_err_t app_config_getFloat(const char* element, float *value){
	return app_config_getValue(element, decimal, value);
}

esp_err_t app_config_getArray(const char* element, char **value){
	esp_err_t err = app_config_getValue(element, array, value);
	ESP_LOGD(TAG, "Found: %s", *value);
	return err;
}

esp_err_t app_config_getString(const char* element, char **value){
	esp_err_t err = app_config_getValue(element, string, value);
	ESP_LOGD(TAG, "Found: %s", *value);
	return err;
}


esp_err_t app_config_init(app_config_cbs_t *cbs){
	//Initializing default NVS partition
	ESP_LOGI(TAG, "Initializing NVS");
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
	    err = nvs_flash_init();
	    if (err != ESP_OK) return err;
	}
	// Opening NVS handle
	ESP_LOGI(TAG, "Opening NVS handle");
	err = nvs_open(CONF_NVS_NAMESPACE, NVS_READWRITE, &app_config_nvs_hanle);
	if (err != ESP_OK) {
	  	nvs_flash_deinit();
	   	return err;
	};
    // Loading stored configuration
	ESP_LOGI(TAG, "Loading configuration");
	err = app_config_load();
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error loading configuration (err %d). Saving current one.", err);
		app_config_save();
		app_config_restart();
	}
	// Starting WiFi
	if (APP_CONFIG_STD_WIFI) {
		ESP_LOGD(TAG, "STD_WIFI defined. Starting WiFi");
		ESP_ERROR_CHECK(app_config_wifi_init());
		ESP_ERROR_CHECK(app_config_http_init());
	}

#ifdef CONFIG_APP_CONFIG_OTA
	if(APP_CONFIG_STD_OTA) {
		ESP_LOGD(TAG, "STD_OTA defined. Starting OTA routines");
		ESP_ERROR_CHECK(app_config_ota_init());
	}
#endif

#ifdef CONFIG_APP_CONFIG_BLUETOOTH_MESH
	// Starting BLE Mesh
	bool config_mesh_enable;
    app_config_getBool("ble_mesh_enable", &config_mesh_enable);
    if (config_mesh_enable){
        ESP_LOGI(TAG, "BLE Mesh enabled");
        esp_err_t err = bluetooth_init();
        if (err) {
            ESP_LOGE(TAG, "bluetooth_init failed (err %d)", err);
            return ESP_FAIL;
        }
		app_config_ble_mesh_init(cbs);
	}
#endif
	return ESP_OK;
}

char *app_config_toJSON(){
	cJSON *json_conf = cJSON_CreateObject();

	if (cJSON_AddStringToObject(json_conf, "name", app_conf.name) == NULL){
       	cJSON_Delete(json_conf);
		return NULL;
    }
	if (cJSON_AddNumberToObject(json_conf, "version", app_conf.version) == NULL){
       	cJSON_Delete(json_conf);
		return NULL;
    }
	
	cJSON *topics = NULL;
	topics = cJSON_AddArrayToObject(json_conf, "topics");
	if (topics == NULL){
   	   	cJSON_Delete(json_conf);
		return NULL;
   	}
	for (uint8_t i=0; i < app_conf.topics_number; i++){
		cJSON *topic = cJSON_CreateObject();
		if (cJSON_AddStringToObject(topic, "short_name", app_conf.topics[i].short_name) == NULL){
	   	   	cJSON_Delete(json_conf);
			return NULL;
        }
		if (cJSON_AddStringToObject(topic, "name", app_conf.topics[i].name) == NULL){
	   	   	cJSON_Delete(json_conf);
			return NULL;
        }
		cJSON *elements = NULL;
		elements = cJSON_AddArrayToObject(topic, "elements");
		if (elements == NULL){
   	   		cJSON_Delete(json_conf);
			return NULL;
   		}
		for (uint8_t j=0; j < app_conf.topics[i].elements_number; j++){
			cJSON *element = cJSON_CreateObject();
			if (cJSON_AddStringToObject(element, "short_name", app_conf.topics[i].elements[j].short_name) == NULL){
	   	   		cJSON_Delete(json_conf);
				return NULL;
        	}
			if (cJSON_AddStringToObject(element, "name", app_conf.topics[i].elements[j].name) == NULL){
	   	   		cJSON_Delete(json_conf);
				return NULL;
        	}
			switch (app_conf.topics[i].elements[j].type){
				case boolean:
					if (cJSON_AddStringToObject(element, "type", "bool") == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
					if (cJSON_AddBoolToObject(element, "value", *(bool *)app_conf.topics[i].elements[j].value) == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
				break;
				case int8:
					if (cJSON_AddStringToObject(element, "type", "int") == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
					if (cJSON_AddNumberToObject(element, "value", *(uint8_t*)app_conf.topics[i].elements[j].value) == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
				break;
				case int16:
					if (cJSON_AddStringToObject(element, "type", "int") == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
					if (cJSON_AddNumberToObject(element, "value", *(uint16_t *)app_conf.topics[i].elements[j].value) == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
				break;
				case int32:
					if (cJSON_AddStringToObject(element, "type", "int") == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
					if (cJSON_AddNumberToObject(element, "value", *(uint32_t *)app_conf.topics[i].elements[j].value) == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
				break;
				case decimal:
					if (cJSON_AddStringToObject(element, "type", "decimal") == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
					if (cJSON_AddNumberToObject(element, "value", *(float *)app_conf.topics[i].elements[j].value) == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
				break;
				case array:
					if (cJSON_AddStringToObject(element, "type", "array") == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
					if (cJSON_AddStringToObject(element, "value", (char *)app_conf.topics[i].elements[j].value) == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
				break;
				case string:
					if (cJSON_AddStringToObject(element, "type", "string") == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
					if (cJSON_AddStringToObject(element, "value", (char *)app_conf.topics[i].elements[j].value) == NULL){
	   			   		cJSON_Delete(json_conf);
						return NULL;
        			}
				break;
				default:
					ESP_LOGE(TAG, "Unknown value type");
   			   		cJSON_Delete(json_conf);
					return NULL;
				break;
			};
			cJSON_AddItemToArray(elements, element);
		}
		cJSON_AddItemToArray(topics, topic);
	}

	char *json_string;
	json_string = cJSON_Print(json_conf);
    if (json_string == NULL)    {
        ESP_LOGE(TAG, "Error generating JSON");
    }
	cJSON_Delete(json_conf);
	return json_string;

}

void app_config_restart(){
	esp_restart();
}

void app_config_erase(){
#ifdef CONFIG_APP_CONFIG_BLUETOOTH_MESH
    esp_ble_mesh_node_local_reset();
#endif
    nvs_erase_all(app_config_nvs_hanle);
    nvs_commit(app_config_nvs_hanle);
	app_config_restart();
}