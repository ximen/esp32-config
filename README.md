# esp32-config
## Disclaimer
This is early pre-alpha piece of software under heavy development. So no guarantees about even compiling can be done for now.
## About
ESP-IDF component which helps to generate some boilerplate code for configuration variables being exposed via web interface.
Almost any project have near the same set of configuration variables such as WiFi, MQTT and other settings.
This component suggested to ease process of maintaining such configuration variables. It stores configuration, exposes it via HTTP server and
applying changes with virtually no code (see Example)
## Usage
1. Create project
1. Create `components` folder if it was not created before
1. Create Git repository for your project (`git init` in project folder)
1. Add component esp32-config as Git submodule: `git submodule add https://github.com/ximen/esp32-config.git` in `components` folder
1. Create file conf.json in project directory containing variables being exported
1. Include `app_config.h` in your `main.c`
1. Build and flash project.
Some technical details such as filnames and paths may be found in coomponent's `Kconfig` file
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
					"type": "boolean"
				}, {
					"name": "String variable",
					"short_name": "string_var",
					"type": "array",
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
    ESP_ERROR_CHECK(app_config_init());		    // Initializing and loading configuration
    char *tmp;
    ESP_ERROR_CHECK(app_config_getArray("string_var", &tmp));
    ESP_LOGI(TAG, "String variable: %s", tmp);
}
```
This will generate basic WiFi settings and expose two variables to configure. All WiFi and HTTP stuff done by component.
