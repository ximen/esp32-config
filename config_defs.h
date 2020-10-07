#include "app_config.h"

static bool	ble_mesh;
static bool	mqtt;
static bool	can_bus;
static uint8_t	sensor1_type;
static char	sensor1_mqtt[20];
static char	sensor1_mesh[4];
static uint8_t	sensor2_type;
static char	sensor2_mqtt[20];
static char	sensor2_mesh[4];
static uint8_t	sensor3_type;
static char	sensor3_mqtt[20];
static char	sensor3_mesh[4];
static uint8_t	zone1_temp;
static uint8_t	zone1_hyst;
static uint8_t	zone1_sensor;
static uint8_t	zone2_temp;
static uint8_t	zone2_hyst;
static uint8_t	zone2_sensor;
static uint8_t	zone3_temp;
static uint8_t	zone3_hyst;
static uint8_t	zone3_sensor;

#define		APP_CONFIG_STD_WIFI	1
APP_CONFIG_DEFINE_STD_WIFI("WiFi", "Thermostat", "")

static app_config_element_t config_protocols_elements[] = {
	APP_CONFIG_DEFINE_BOOL(ble_mesh, "Bluetooth Mesh"),
	APP_CONFIG_DEFINE_BOOL(mqtt, "MQTT"),
	APP_CONFIG_DEFINE_BOOL(can_bus, "CAN"),
};


#define		APP_CONFIG_STD_BLE_MESH	1

#define		APP_CONFIG_STD_MQTT	1
APP_CONFIG_DEFINE_STD_MQTT("MQTT", 1883)

static app_config_element_t config_sensors_elements[] = {
	APP_CONFIG_DEFINE_INT8(sensor1_type, "Sensor 1 type"),
	APP_CONFIG_DEFINE_ARRAY(sensor1_mqtt, "Sensor 1 MQTT topic"),
	APP_CONFIG_DEFINE_ARRAY(sensor1_mesh, "Sensor 1 Mesh Address"),
	APP_CONFIG_DEFINE_INT8(sensor2_type, "Sensor 2 type"),
	APP_CONFIG_DEFINE_ARRAY(sensor2_mqtt, "Sensor 2 MQTT topic"),
	APP_CONFIG_DEFINE_ARRAY(sensor2_mesh, "Sensor 2 Mesh Address"),
	APP_CONFIG_DEFINE_INT8(sensor3_type, "Sensor 3 type"),
	APP_CONFIG_DEFINE_ARRAY(sensor3_mqtt, "Sensor 3 MQTT topic"),
	APP_CONFIG_DEFINE_ARRAY(sensor3_mesh, "Sensor 3 Mesh Address"),
};


static app_config_element_t config_zones_elements[] = {
	APP_CONFIG_DEFINE_INT8(zone1_temp, "Zone 1 temperature"),
	APP_CONFIG_DEFINE_INT8(zone1_hyst, "Zone 1 hysteresis"),
	APP_CONFIG_DEFINE_INT8(zone1_sensor, "Zone 1 sensor"),
	APP_CONFIG_DEFINE_INT8(zone2_temp, "Zone 2 temperature"),
	APP_CONFIG_DEFINE_INT8(zone2_hyst, "Zone 2 hysteresis"),
	APP_CONFIG_DEFINE_INT8(zone2_sensor, "Zone 2 sensor"),
	APP_CONFIG_DEFINE_INT8(zone3_temp, "Zone 3 temperature"),
	APP_CONFIG_DEFINE_INT8(zone3_hyst, "Zone 3 hysteresis"),
	APP_CONFIG_DEFINE_INT8(zone3_sensor, "Zone 3 sensor"),
};

static app_config_topic_t conf_topics[] = {
	APP_CONFIG_DEFINE_TOPIC(std_wifi_topic , "WiFi", config_std_wifi_elements),
	APP_CONFIG_DEFINE_TOPIC(protocols, "Protocols", config_protocols_elements),
	APP_CONFIG_DEFINE_TOPIC(std_mqtt_topic , "MQTT", config_std_mqtt_elements),
	APP_CONFIG_DEFINE_TOPIC(sensors, "Sensors", config_sensors_elements),
	APP_CONFIG_DEFINE_TOPIC(zones, "Zones", config_zones_elements),
};

static app_config_t app_conf = { 1, "Thermostat configuration", "thermo_config", 6, conf_topics };