/*
 * app_wifi.c
 *
 *  Created on: 19 мая 2020 г.
 *      Author: ximen
 */
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "string.h"
#include "config.h"
#include "esp_wifi.h"
#include "app_config_wifi.h"
#include "sdkconfig.h"
#include "config_defs.h"

#define TAG "APP_CONFIG_WIFI"
#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t	s_wifi_event_group;
static int s_retry_num = 0;

void wifi_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
	if (event_base == WIFI_EVENT){
		switch(event_id){
			case WIFI_EVENT_AP_STACONNECTED:
				ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
						MAC2STR(((wifi_event_ap_staconnected_t *)event_data)->mac),
						((wifi_event_ap_staconnected_t *)event_data)->aid);
			break;
			case WIFI_EVENT_AP_STADISCONNECTED:
				ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
						MAC2STR(((wifi_event_ap_staconnected_t *)event_data)->mac),
						((wifi_event_ap_staconnected_t *)event_data)->aid);
			break;
			case WIFI_EVENT_STA_START:
				esp_wifi_connect();
			break;
			case WIFI_EVENT_STA_DISCONNECTED:
				if(s_retry_num < APP_CONFIG_WIFI_MAXIMUM_RETRIES){
					esp_wifi_connect();
					xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
					s_retry_num++;
					ESP_LOGI(TAG, "Retrying to connect to the AP");
				}
				ESP_LOGE(TAG, "Connection to the AP failed");
				esp_wifi_connect();
			break;
			default:
			break;
		}
	} else if (event_base == IP_EVENT){
		if (event_id == IP_EVENT_STA_GOT_IP){
			ESP_LOGI(TAG, "Got ip: %s", ip4addr_ntoa((ip4_addr_t *)&((ip_event_got_ip_t *)event_data)->ip_info.ip));
			s_retry_num = 0;
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		}
	}
}

void app_config_wifi_init_ap(){
	ESP_LOGI(TAG, "Starting WiFi AP");
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_ap();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, NULL));
	char *ssid = "";
	char *pass = "";
	ESP_ERROR_CHECK(app_config_getArray("std_wifi_ssid", &ssid));
	ESP_ERROR_CHECK(app_config_getArray("std_wifi_psk", &pass));
	wifi_config_t wifi_config = {
			.ap = {
					.ssid_len = strlen(ssid),
					.max_connection = 1,
					.authmode = WIFI_AUTH_WPA2_PSK
			},
	};
	strncpy((char *)wifi_config.ap.ssid, ssid, APP_CONFIG_MAX_SSID_LEN);
	strncpy((char *)wifi_config.ap.password, pass, APP_CONFIG_MAX_PSK_LEN);
	if (strlen(pass) == 0) wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "WiFi AP started.");
}

void app_config_wifi_init_sta(){
	ESP_LOGI(TAG, "Starting WiFi STA");	
	s_wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler, NULL));
	char *ssid = "";
	char *pass = "";
	ESP_ERROR_CHECK(app_config_getArray("std_wifi_ssid", &ssid));
	ESP_ERROR_CHECK(app_config_getArray("std_wifi_psk", &pass));
	wifi_config_t wifi_config;
	strncpy((char *)wifi_config.sta.ssid, ssid, APP_CONFIG_MAX_SSID_LEN);
	strncpy((char *)wifi_config.sta.password, pass, APP_CONFIG_MAX_PSK_LEN);
	ESP_LOGI(TAG, "SSID: %s, PSK: %s", wifi_config.ap.ssid, wifi_config.ap.password);
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "WiFi STA started");
}

esp_err_t app_config_wifi_init(){
	if (!APP_CONFIG_STD_WIFI) {
		ESP_LOGE(TAG, "STD_WIFI element not defined");
		return ESP_ERR_NOT_FOUND;
	}
	esp_err_t err;
	bool ap;
	err = app_config_getBool("std_wifi_ap", &ap);
	if(err){
		ESP_LOGE(TAG, "Error getting AP status. Aborting WiFi init (err %d)", err);
		return ESP_ERR_NOT_FOUND;
	}
	if (ap)	app_config_wifi_init_ap();
	else app_config_wifi_init_sta();
	return ESP_OK;
}
