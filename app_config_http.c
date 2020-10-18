/*
 * app_http.c
 *
 *  Created on: 19 сент. 2020 г.
 *      Author: ximen
 */

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include "app_config.h"
#include "sdkconfig.h"
#include "config_html.h"
#include <esp_http_server.h>

#define TAG "APP_CONFIG_HTTP"

httpd_handle_t app_config_http_server;

esp_err_t get_conf_handler(httpd_req_t *req){
    char *json_conf = app_config_toJSON();
    if (json_conf == NULL){
        httpd_resp_send_500(req);
        return ESP_FAIL;
    } else {
        httpd_resp_send(req, json_conf, HTTPD_RESP_USE_STRLEN);
        free(json_conf);
        return ESP_OK;
    }
}

esp_err_t get_html_handler(httpd_req_t *req){
    httpd_resp_send(req, app_config_html, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

esp_err_t post_conf_handler(httpd_req_t *req){
    ESP_LOGI(TAG, "Post handler triggered");
    httpd_resp_send_500(req);    
	return ESP_OK;
}

httpd_uri_t uri_get_html = {
    .uri      = CONFIG_APP_CONFIG_HTML_GET_PATH,
    .method   = HTTP_GET,
    .handler  = get_html_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get = {
    .uri      = CONFIG_APP_CONFIG_REST_PATH,
    .method   = HTTP_GET,
    .handler  = get_conf_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_post = {
    .uri      = CONFIG_APP_CONFIG_HTML_SET_PATH,
    .method   = HTTP_POST,
    .handler  = post_conf_handler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t start_webserver(void){
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;
    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_get_html);        
        httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(){
    if (app_config_http_server) {
        /* Stop the httpd server */
        httpd_stop(app_config_http_server);
    }
}

esp_err_t app_config_http_init(){
    app_config_http_server = start_webserver();
	return ESP_OK;
}
