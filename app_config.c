/*
 * config.c
 *
 *  Created on: 18 мая 2020 г.
 *      Author: ximen
 */
#include "app_config.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "string.h"
#include "config_defs.h"
#include "cJSON.h"

#define TAG "APP_CONFIG"
static nvs_handle_t app_config_nvs_hanle;
static app_config_t app_conf;

// Returns configuration singleton
app_config_t *app_config_get(){
	return &app_conf;
}

esp_err_t app_config_load_element(app_config_element_t *element){
	ESP_LOGI(TAG, "Loading element %s with size %d", element->short_name, element->size);
	esp_err_t err = ESP_OK;
	switch(element->type){
	case boolean:		// Boolean storead as uint8_t
		err = nvs_get_u8(app_config_nvs_hanle, element->short_name, element->value);
		ESP_LOGD(TAG, "Got %d", *(uint8_t *)element->value);
		break;
	case array:
		err = nvs_get_blob(app_config_nvs_hanle, element->short_name, element->value, &element->size);
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
	default:
		break;
	}
	return err;
}

esp_err_t app_config_load_topic(app_config_topic_t *topic){
	ESP_LOGI(TAG, "Loading topic %s with %d elements", topic->short_name, topic->elements_number);
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
	ESP_LOGI(TAG, "Saving element %s with size %d", element->short_name, element->size);
	esp_err_t err = ESP_OK;
	switch(element->type){
	case boolean:		// Boolean storead as uint8_t
		err = nvs_set_u8(app_config_nvs_hanle, element->short_name, *(uint8_t *)element->value);
		break;
	case array:
		err = nvs_set_blob(app_config_nvs_hanle, element->short_name, element->value, element->size);
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

esp_err_t app_config_getValue(const char* element, uint8_t type, void *value){
	ESP_LOGI(TAG, "Getting from config: %s", element);
	app_config_element_t *elt = findElement(element);
	if(elt != NULL){
		if(elt->type != type) return ESP_ERR_INVALID_ARG;
		if(type == boolean) *(bool *)value = *(bool *)elt->value;
		ESP_LOGI(TAG, "Found: %s", (char *)elt->value);
		if(type == array) *(char **)value = (char *)elt->value;
		return ESP_OK;
	} else {
		return ESP_ERR_NOT_FOUND;
	}
}

esp_err_t app_config_getBool(const char* element, bool *value){
	return app_config_getValue(element, boolean, value);
}

esp_err_t app_config_getArray(const char* element, char **value){
	esp_err_t err = app_config_getValue(element, array, value);
	ESP_LOGI(TAG, "Found: %s", *value);
	return err;
}


esp_err_t app_config_init(){
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
		ESP_LOGE(TAG, "Error loading configuration (err %d)", err);
	}
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