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
print ("Parsing BLE Mesh JSON configurtion file " + json_filename)
h_filename = sys.argv[2] + "/" + 'app_config_ble_mesh.c'                                                 # Output HTML include file

# Open files
with open(json_filename, "r") as json_file:
    conf = json.load(json_file)
h_file = open(h_filename, 'w')

inc_config_includes = '''#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_common_api.h"
#include "app_config_ble_mesh.h"

#define TAG "APP_CONFIG_BLE_MESH"
#define CID_ESP 0x02E5

static uint8_t dev_uuid[16] = { 0xdd, 0xdd };
'''

# Configuration server declaration
inc_config_server_declaration = '''static esp_ble_mesh_cfg_srv_t ble_mesh_config_server = {
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
    /* 3 transmissions with 20ms interval */
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

'''
# On/Off server declaration
inc_config_onoff_server_def = '''ESP_BLE_MESH_MODEL_PUB_DEFINE({1}, 2 + 3, ROLE_NODE);
static esp_ble_mesh_gen_onoff_srv_t {0} = {{
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
}};

'''

inc_config_onoff_server_arr = 'ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&{1}, &{0}),\n'
inc_config_config_server_arr = 'ESP_BLE_MESH_MODEL_CFG_SRV(&ble_mesh_config_server),\n'
inc_config_server_elt = '\tESP_BLE_MESH_ELEMENT(0, {0}, {1}),\n'
inc_config_element_array = '''static esp_ble_mesh_model_t {0}[] = {{
    {1}
}};

'''

inc_config_elements = '''static esp_ble_mesh_elem_t ble_mesh_elements[] = {{
{0}
}};

'''

inc_config_composition = '''static esp_ble_mesh_comp_t ble_mesh_composition = {
    .cid = CID_ESP,
    .elements = ble_mesh_elements,
    .element_count = ARRAY_SIZE(ble_mesh_elements),
};

'''

inc_config_provision = '''static esp_ble_mesh_prov_t ble_mesh_provision = {
    .uuid = dev_uuid,
    .output_size = 0,
    .output_actions = 0,
};

'''
inc_config_bt_init = '''esp_err_t bluetooth_init(void)
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed", __func__);
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed", __func__);
        return ret;
    }

    return ret;
}

'''
inc_config_init = '''esp_err_t app_config_ble_mesh_init()
{
    esp_err_t err;

    err = esp_ble_mesh_init(&ble_mesh_provision, &ble_mesh_composition);
    if (err) {
        ESP_LOGE(TAG, "Initializing mesh failed (err %d)", err);
        return err;
    }

    esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);

    ESP_LOGI(TAG, "BLE Mesh Node initialized");

    return err;
}

'''
output = ""
models_def = inc_config_server_declaration
models_arrays = ""
elements_array = inc_config_elements
elements_models = ""

# Defining models
element_num = 0
for element in conf["elements"]:
    element_sig_models = ""
    element_vnd_models = ""
    sig_models = element.get("sig_models")
    if element.get("root"):
        element_sig_models += inc_config_config_server_arr
    if sig_models:
        print ("Found sig models")
        for model in sig_models:
            if model["model"] == "generic_on_off_server":
                print ("Generic ON/OFF server found: {0}".format(model["short_name"]))
                models_def         += inc_config_onoff_server_def.format(model["short_name"], model["short_name"] + "_pub")
                element_sig_models += inc_config_onoff_server_arr.format(model["short_name"], model["short_name"] + "_pub")
            else:
                print ("Unsupported model")
    element_sig_models_arr_name = "ble_mesh_element_" + str(element_num) + "_sig_models"
    element_vnd_models_arr_name = "ble_mesh_element_" + str(element_num) + "_vnd_models"
    models_arrays += inc_config_element_array.format(element_sig_models_arr_name, element_sig_models)
    elements_models += inc_config_server_elt.format(element_sig_models_arr_name if element_sig_models else "ESP_BLE_MESH_MODEL_NONE", element_vnd_models_arr_name if element_vnd_models else "ESP_BLE_MESH_MODEL_NONE")
    element_num += 1

# Writing result
output = inc_config_includes + \
    models_def + \
    models_arrays + \
    elements_array.format(elements_models) + \
    inc_config_composition + \
    inc_config_provision + \
    inc_config_bt_init + \
    inc_config_init
    
h_file.write(output)