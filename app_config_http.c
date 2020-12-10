/*
 * app_http.c
 *
 *  Created on: 19 сент. 2020 г.
 *      Author: ximen
 */

#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "app_config.h"
#include "sdkconfig.h"
#include "config_html.h"
#include <esp_http_server.h>

#define TAG "APP_CONFIG_HTTP"

httpd_handle_t app_config_http_server;

void urldecode(char *dst, const char *src){
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

esp_err_t app_http_get_bool_value(char *string, char *name, bool *value){
    ESP_LOGD(TAG, "Getting boolean value %s\n", name);
    ESP_LOGD(TAG, "Allocating %d bytes\n", strlen(string));
    char *buf = malloc(strlen(string) + 1);
    if (!buf) {
        ESP_LOGE(TAG, "Error allocating buffer. Get bool failed.");
        return ESP_ERR_NO_MEM;
    }
    strcpy(buf, string);
    char *end_str;
    char* token = strtok_r(buf, "&", &end_str); 
    while (token) {
        char *end_token;
        char* key = strtok_r(token, "=", &end_token); 
        char *val = strtok_r(NULL, "=", &end_token);
        if(!val){
            val="";
        }
        token = strtok_r(NULL, "&", &end_str);
        if(!strcmp(name, key)){
            if(!strcmp(val, "on")){
                *value = true;
                ESP_LOGD(TAG, "Found: true\n");
                free(buf);
                return ESP_OK;
            }
        }
    }
    *value = false;
    free(buf);
    ESP_LOGD(TAG, "Not found: false\n");
    return ESP_OK;
}

esp_err_t app_http_get_int_value(char *string, char *name, int32_t *value){
    ESP_LOGD(TAG, "Getting int value %s\n", name);
    char *buf = malloc(strlen(string) + 1);
    if (!buf) {
        ESP_LOGE(TAG, "Error allocating buffer. Get int failed.");
        return ESP_ERR_NO_MEM;
    }
    strcpy(buf, string);
    char *end_str;
    char* token = strtok_r(buf, "&", &end_str); 
    while (token) {
        char *end_token;
        char* key = strtok_r(token, "=", &end_token); 
        char *val = strtok_r(NULL, "=", &end_token);
        if(!val){
            val="";
        }
        token = strtok_r(NULL, "&", &end_str);
        if(!strcmp(name, key)){
            int32_t v = (int32_t)strtol(val, (char **)NULL, 10);
            ESP_LOGD(TAG, "Found: %d\n", v);
            *value = v;
            free(buf);
            return ESP_OK;
        }
    }
    free(buf);
    ESP_LOGD(TAG, "Not found");
    return ESP_ERR_NOT_FOUND;
}

esp_err_t app_http_get_string_value(char *string, char *name, char *value){
    ESP_LOGD(TAG, "Getting string value %s\n", name);
    char *buf = malloc(strlen(string) + 1);
    if (!buf) {
        ESP_LOGE(TAG, "Error allocating buffer. Get string failed.");
        return ESP_ERR_NO_MEM;
    }
    urldecode(buf, string);
    //strcpy(buf, string);
    char *end_str;
    char *token = strtok_r(buf, "&", &end_str); 
    while (token) {
        char *end_token;
        char* key = strtok_r(token, "=", &end_token); 
        char *val = strtok_r(NULL, "=", &end_token);
        if (!val){
            val = "";
        }
        token = strtok_r(NULL, "&", &end_str);
        if(!strcmp(name, key)){
            ESP_LOGD(TAG, "Found: %s\n", val);
            strcpy(value, val);
            free(buf);
            return ESP_OK;
        }
    }
    free(buf);
    ESP_LOGD(TAG, "Not found");
    return ESP_ERR_NOT_FOUND;
}

esp_err_t get_conf_handler(httpd_req_t *req){
    ESP_LOGI(TAG, "Sending JSON config");
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
    ESP_LOGI(TAG, "Replying HTML");
    httpd_resp_send(req, app_config_html, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

esp_err_t post_conf_handler(httpd_req_t *req){
    ESP_LOGI(TAG, "Parsing new config");
    ESP_LOGD(TAG, "Allocating %d bytes", sizeof(char)*req->content_len);
    char *buf = malloc(sizeof(char)*req->content_len + 1);
    if(buf == NULL) {
        ESP_LOGE(TAG, "Error allocating buffer. Parsing POST request failed.");
        return ESP_ERR_NO_MEM;
    }
    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        free(buf);
        return ESP_FAIL;
    }
    buf[req->content_len] = 0;
    /* Send a simple response */
    //const char resp[] = "URI POST Response";
    app_config_t *app_conf = app_config_get();
    ESP_LOGD(TAG, "Topic number %d", app_conf->topics_number);
	for (uint8_t i = 0; i < app_conf->topics_number; i++){
        ESP_LOGD(TAG, "Topic %s, Elements number %d", app_conf->topics[i].short_name, app_conf->topics[i].elements_number);
        for (uint8_t j = 0; j < app_conf->topics[i].elements_number; j++){
            ESP_LOGD(TAG, "Number %d, j=%d", app_conf->topics[i].elements_number, j);
            ESP_LOGD(TAG, "Looking for %s", app_conf->topics[i].elements[j].short_name);
            if (app_conf->topics[i].elements[j].type == boolean){
                bool tmp = false;
                app_http_get_bool_value(buf, app_conf->topics[i].elements[j].short_name, &tmp);
                ESP_LOGD(TAG, "Got %d", tmp);
                app_config_setValue(app_conf->topics[i].elements[j].short_name, &tmp);
            } else if (app_conf->topics[i].elements[j].type == int8){
                int32_t tmp;
                app_http_get_int_value(buf, app_conf->topics[i].elements[j].short_name, &tmp);
                ESP_LOGD(TAG, "Got %d", tmp);
                int8_t tmp8 = (int8_t)tmp;
                app_config_setValue(app_conf->topics[i].elements[j].short_name, &tmp8);
            } else if (app_conf->topics[i].elements[j].type == int16){
                int32_t tmp;
                app_http_get_int_value(buf, app_conf->topics[i].elements[j].short_name, &tmp);
                ESP_LOGD(TAG, "Got %d", tmp);
                int16_t tmp16 = (int16_t)tmp;
                app_config_setValue(app_conf->topics[i].elements[j].short_name, &tmp16);
            } else if (app_conf->topics[i].elements[j].type == int32){
                int32_t tmp;
                app_http_get_int_value(buf, app_conf->topics[i].elements[j].short_name, &tmp);
                ESP_LOGD(TAG, "Got %d", tmp);
                app_config_setValue(app_conf->topics[i].elements[j].short_name, &tmp);
            } else if (app_conf->topics[i].elements[j].type == array){
                char tmp[100];
                app_http_get_string_value(buf, app_conf->topics[i].elements[j].short_name, tmp);
                ESP_LOGD(TAG, "Got %s", tmp);
                app_config_setValue(app_conf->topics[i].elements[j].short_name, tmp);
            } else if (app_conf->topics[i].elements[j].type == string){
                char tmp[100];
                app_http_get_string_value(buf, app_conf->topics[i].elements[j].short_name, tmp);
                ESP_LOGD(TAG, "Got %s", tmp);
                app_config_setValue(app_conf->topics[i].elements[j].short_name, tmp);
            } else {

            }
        }
	}
    ESP_LOGI(TAG, "Saving new configuration");
    app_config_save();
    app_config_restart();
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    free(buf);
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


