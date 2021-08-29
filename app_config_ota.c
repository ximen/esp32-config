#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_err.h"
#include "app_config_wifi.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#define TAG "APP_CONFIG_OTA"

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

TaskHandle_t ota_task_handle;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

void ota_task(void *pvParameter){
    ESP_LOGI(TAG, "Starting OTA task");
    char *ota_url;
    app_config_getString("std_ota_url", &ota_url);
    esp_http_client_config_t config = {
        .url = ota_url,
        .cert_pem = (char *)server_cert_pem_start,
        .event_handler = _http_event_handler};
    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware upgrade failed");
    }
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void ota_ip_cb(ip_event_t event, void *event_data){
    if(event == IP_EVENT_STA_GOT_IP){
        bool std_ota_enable;
        app_config_getBool("std_ota_enable", &std_ota_enable);
        char *std_ota_url;
        app_config_getString("std_ota_url", &std_ota_url);
        if(std_ota_enable && strlen(std_ota_url)){
            //esp_wifi_set_ps(WIFI_PS_NONE);
            // Start OTA thread
            xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, &ota_task_handle);
        }
    } else if (event == IP_EVENT_STA_LOST_IP){
        //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
        // Stop OTA thread
        vTaskDelete(ota_task_handle);
    }
}

esp_err_t app_config_ota_init(){
    app_config_ip_register_cb(IP_EVENT_STA_GOT_IP, ota_ip_cb);
    app_config_ip_register_cb(IP_EVENT_STA_LOST_IP, ota_ip_cb);
    return ESP_OK;
}