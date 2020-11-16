#!/usr/bin/python3

import json
import sys
from configparser import ConfigParser

if len(sys.argv) != 3:
    print ("Error arguments count. Exiting.")
    sys.exit(-1)

sdkconfig = ConfigParser()
with open(sys.argv[1] + "/sdkconfig") as stream:
    sdkconfig.read_string("[sdk]\n" + stream.read())

json_filename = sys.argv[1] + "/" + sdkconfig['sdk']['CONFIG_APP_CONFIG_BLE_MESH_FILE_NAME'].strip('"')  # Configurations JSON file
h_filename = sys.argv[2] + "/" + 'config_ble_mesh.h'                                                     # Output HTML include file

out_models_declare = ""
out_models = ""

def add_onoff_server_model(short_name):
    out_models_declare += "ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_0, 2 + 3, ROLE_NODE);\n"
    out_models_declare += "static esp_ble_mesh_gen_onoff_srv_t onoff_server_0 = {\n"
    out_models_declare += "\t.rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,\n"
    out_models_declare += "\t.rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,\n};\n\n"

def add_root_models(element):
    out_models += "static esp_ble_mesh_model_t root_models[] = {\n"
    out_models += "ESP_BLE_MESH_MODEL_CFG_SRV(&app_config_ble_config_server),\n"

# Open files
with open(json_filename, "r") as json_file:
    conf = json.load(json_file)

# Create includes
out_headers = '''
#include <stdio.h>
#include \"esp_log.h
#include \"esp_ble_mesh_provisioning_api.h
#include \"esp_ble_mesh_config_model_api.h
#include \"esp_ble_mesh_generic_model_api.h
'''
# Create configuration server
out_config_server = '''
static esp_ble_mesh_cfg_srv_t app_config_ble_config_server = {
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};'''


root_elt = ""
# Declaring models
for element in conf["elements"]:
    if element.get("root") == True:
        add_root_models(element)
        out_models += "};\n"
        add_models()
        if element.get("models") == True:
            for model in element["sig_models"]:
                if model["model"] == "generic_on_off_server":
                    root_elt += "ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_0, &onoff_server_0),\n"
                    add_onoff_server_model(model["short_name"])
                else:
                    print("Unsupported model")
    else:


# Write output
output = '''
{0}
'''.format(out_headers)

h_file = open(h_filename, 'w')
h_file.write(output)