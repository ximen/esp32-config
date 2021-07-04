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
static app_config_wifi_cb_arg_t wifi_cbs[] = {
	{WIFI_EVENT_WIFI_READY, NULL},
	{WIFI_EVENT_SCAN_DONE, NULL},
	{WIFI_EVENT_STA_START, NULL},
	{WIFI_EVENT_STA_STOP, NULL},
	{WIFI_EVENT_STA_CONNECTED, NULL},
	{WIFI_EVENT_STA_DISCONNECTED, NULL},
	{WIFI_EVENT_STA_AUTHMODE_CHANGE, NULL},
	{WIFI_EVENT_STA_WPS_ER_SUCCESS, NULL},
	{WIFI_EVENT_STA_WPS_ER_FAILED, NULL},
	{WIFI_EVENT_STA_WPS_ER_TIMEOUT, NULL},
	{WIFI_EVENT_STA_WPS_ER_PIN, NULL},
	{WIFI_EVENT_AP_START, NULL},
	{WIFI_EVENT_AP_STOP, NULL},
	{WIFI_EVENT_AP_STACONNECTED, NULL},
	{WIFI_EVENT_AP_STADISCONNECTED, NULL},
	{WIFI_EVENT_AP_PROBEREQRECVED, NULL}
};
static app_config_ip_cb_arg_t ip_cbs[] = {
	{IP_EVENT_STA_GOT_IP, NULL},
	{IP_EVENT_STA_LOST_IP, NULL},
	{IP_EVENT_AP_STAIPASSIGNED, NULL},
	{IP_EVENT_GOT_IP6, NULL},
	{IP_EVENT_ETH_GOT_IP, NULL}
};

void run_wifi_cb(wifi_event_t event, void *event_data){
	for(uint8_t i = 0; i < SIZEOF(wifi_cbs); i++)
		if(wifi_cbs[i].event == event && wifi_cbs[i].cb) wifi_cbs[i].cb(event, event_data);
}

void run_ip_cb(ip_event_t event, void *event_data){
	for(uint8_t i = 0; i < SIZEOF(ip_cbs); i++)
		if(ip_cbs[i].event == event && ip_cbs[i].cb) ip_cbs[i].cb(event, event_data);
}

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
		run_wifi_cb(event_id, event_data);
	} else if (event_base == IP_EVENT){
		if (event_id == IP_EVENT_STA_GOT_IP){
			ESP_LOGI(TAG, "Got ip: %s", ip4addr_ntoa((ip4_addr_t *)&((ip_event_got_ip_t *)event_data)->ip_info.ip));
			s_retry_num = 0;
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		}
		run_ip_cb(event_id, event_data);
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
	ESP_ERROR_CHECK(app_config_getString("std_wifi_ssid", &ssid));
	ESP_ERROR_CHECK(app_config_getString("std_wifi_psk", &pass));
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
	esp_netif_create_default_wifi_sta();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler, NULL));
	char *ssid = "";
	char *pass = "";
	ESP_ERROR_CHECK(app_config_getString("std_wifi_ssid", &ssid));
	ESP_ERROR_CHECK(app_config_getString("std_wifi_psk", &pass));
	wifi_config_t wifi_config = {};
	strcpy((char *)wifi_config.sta.ssid, ssid);
	strcpy((char *)wifi_config.sta.password, pass);
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	wifi_config.sta.pmf_cfg.capable = true;
	wifi_config.sta.pmf_cfg.required = false;
	ESP_LOGI(TAG, "SSID: %s, PSK: %s", wifi_config.sta.ssid, wifi_config.sta.password);
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "WiFi STA started");
}

esp_err_t app_config_wifi_init(){
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

esp_err_t app_config_wifi_register_cb(wifi_event_t event, app_config_wifi_cb_t cb){
	for(uint8_t i = 0; i < SIZEOF(wifi_cbs); i++){
		if(wifi_cbs[i].event == event){
			wifi_cbs[i].cb = cb;
			return ESP_OK;
		}
	}
	return ESP_ERR_NOT_FOUND;
}

esp_err_t app_config_ip_register_cb(ip_event_t event, app_config_ip_cb_t cb){
	for(uint8_t i = 0; i < SIZEOF(ip_cbs); i++){
		if(ip_cbs[i].event == event){
			ip_cbs[i].cb = cb;
			return ESP_OK;
		}
	}
	return ESP_ERR_NOT_FOUND;
}