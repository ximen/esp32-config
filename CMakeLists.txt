set(srcs  "app_config.c" 
          "app_config_wifi.c" 
          "app_config_http.c" 
          "app_config_mqtt.c" 
          "app_config_mqtt_switch.c" 
          "app_config_mqtt_light.c" 
          "app_config_mqtt_binary_sensor.c")
set(reqs "nvs_flash" "esp_http_server" "json" "mqtt" "bt" "esp_http_client" "esp_https_ota")

if(CONFIG_APP_CONFIG_BLUETOOTH_MESH)
     list(APPEND srcs "app_config_ble_mesh.c")
     list(APPEND reqs bt)
endif(CONFIG_APP_CONFIG_BLUETOOTH_MESH)

if(CONFIG_APP_CONFIG_OTA)
     list(APPEND srcs "app_config_ota.c")
     list(APPEND reqs "esp_http_client" "esp_https_ota")
endif(CONFIG_APP_CONFIG_OTA)

if(CONFIG_APP_CONFIG_OTA)
     idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include"
                       REQUIRES ${reqs}
                       EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)
else()
     idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
                    REQUIRES ${reqs})
endif(CONFIG_APP_CONFIG_OTA)

if(NOT CMAKE_BUILD_EARLY_EXPANSION)

add_custom_target(config_defs 
     COMMENT "Generation configuration includes..."
     COMMAND python ${COMPONENT_DIR}/generate_config.py ${PROJECT_DIR} ${CONFIG_APP_CONFIG_FILE_NAME} ${COMPONENT_DIR})
add_custom_target(config_html 
     COMMENT "Generation HTML includes..."
     COMMAND python ${COMPONENT_DIR}/generate_html.py ${PROJECT_DIR} ${COMPONENT_DIR})

if(CONFIG_APP_CONFIG_BLUETOOTH_MESH)     
     add_custom_target(config_ble_mesh
          COMMENT "Generation Bluetooth Mesh includes..."
          COMMAND python ${COMPONENT_DIR}/generate_ble_mesh.py ${PROJECT_DIR} ${COMPONENT_DIR})
     add_dependencies(${COMPONENT_LIB} config_ble_mesh)
endif(CONFIG_APP_CONFIG_BLUETOOTH_MESH)

add_dependencies(${COMPONENT_LIB} config_defs config_html)

set_property(DIRECTORY "${COMPONENT_DIR}" APPEND PROPERTY
     ADDITIONAL_MAKE_CLEAN_FILES config_defs.h config.html config_html.h app_config_ble_mesh.c)
endif(NOT CMAKE_BUILD_EARLY_EXPANSION)                        