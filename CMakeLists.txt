idf_component_register(SRCS "app_config.c" "app_config_wifi.c" "app_config_http.c"
                       INCLUDE_DIRS "include"
                       REQUIRES nvs_flash esp_http_server json)

if(NOT CMAKE_BUILD_EARLY_EXPANSION) 

add_custom_target(config_defs 
     COMMAND python ${COMPONENT_DIR}/generate_config.py ${PROJECT_DIR} ${CONFIG_APP_CONFIG_FILE_NAME} ${COMPONENT_DIR})
add_custom_target(config_html 
     COMMAND python ${COMPONENT_DIR}/generate_html.py ${PROJECT_DIR} ${COMPONENT_DIR})

add_dependencies(${COMPONENT_LIB} config_defs config_html)

set_property(DIRECTORY "${COMPONENT_DIR}" APPEND PROPERTY
     ADDITIONAL_MAKE_CLEAN_FILES config_defs.h config.html config_html.h)
endif()                        