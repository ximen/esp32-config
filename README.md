# esp32-config
## Disclaimer
This is early pre-alpha piece of software under heavy development. So no guarantees about even compiling can be done for now.
## About
ESP-IDF component which helps to generate some boilerplate code for configuration variables being exposed via web interface.
Almost any project have near the same set of configuration variables such as WiFi, MQTT and other settings.
This component suggested to ease process of maintaining such configuration variables. It stores configuration, exposes it via HTTP server and
applying changes with virtually no code (see Example).
Also this library contains some API simplification for frequently used code such as BLE Mesh, MQTT and other.
## Basic usage
1. Create project
1. Create `components` folder if it was not created before
1. Create empty Git repository for your project (`git init` in project folder)
1. Add component esp32-config as Git submodule: `git submodule add https://github.com/ximen/esp32-config.git` in `components` folder
1. Create file conf.json in project directory containing variables being exported
1. Include `app_config.h` in your `main.c`
1. Build and flash project.
Some technical details such as filenames and paths may be found in component's `Kconfig` file.
Sample projects using this components are [Thermostat](https://github.com/ximen/thermostat) and [Switch](https://github.com/ximen/switch)
## Example
> conf.json:
```json
{
	"name": "Example configuration",
	"short_name": "example_config",
	"version": 1,
	"topics": [
		{
			"name": "WiFi",
			"std_wifi": true,
			"default_ssid": "Test",
			"default_psk": ""
		}, {
            "name": "Variables",
			"short_name": "vars_topic",
			"elements": [
				{
					"name": "Boolean variable",
					"short_name": "bool_var",
					"type": "boolean",
					"default": false
				}, {
					"name": "String variable",
					"short_name": "string_var",
					"type": "string",
                    "size": "10",
                    "depends_on": "bool_var"
				}]      
        }
    ]
}
```
> main.c:
```c
#include "esp_err.h"
#include "esp_log.h"
#include "app_config.h"

#define TAG "MAIN"

void app_main(void){
	app_config_cbs_t app_cbs;						// Structure containing pointers to callback functions

    ESP_ERROR_CHECK(app_config_init(&app_cbs));		// Initializing and loading configuration
    char *tmp;
    ESP_ERROR_CHECK(app_config_getArray("string_var", &tmp));
    ESP_LOGI(TAG, "String variable: %s", tmp);
}
```
This will generate basic WiFi settings and expose two variables to configure. All WiFi and HTTP stuff done by component internally.

## conf.json
`conf.json` contains object defining device's configuration (see example above).
* `name` field define device name. This name mostly used as header in web-interface.
* `short_name` is technical field. Maximum field length deined in `Kconfig` file.
* `version` configuration version to maintain backword compatibility
* `topics` array contains all configuration topics objects. Each topic has own name and contains array of elements, representing wxposed variables.

All JSON objects, except `std_*` topics, must contain `name` and `short_name` fields. For `std_*` topics `short_name` being generated internally.

Available standard topics:
* `std_wifi` for basic WiFi configuration:
```json
{
	"name": "WiFi",
    "std_wifi": true,
    "default_ssid": "Switch_x10",
    "default_psk": ""
}
```
* `std_mqtt` for MQTT
```json
{
	"name": "MQTT",
    "std_mqtt": true
}
```

Minimum topic object must contain:
```json
{
	"name": "Topic name",
	"short_name": "topic_name_topic",
	"elements": []      
}
```

Available element types are:
* `bool`
* `array` 
* `string`
* `int8`
* `int16`
* `int32`

`array` and `string` elements must contain `size` field to determine its maximum length.

All elements support `default` value.

Topics and elements may contain `depends_on` field to control it's visibility according specified boolean element's value.