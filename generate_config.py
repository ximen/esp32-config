#!/usr/bin/python3

import json
import sys

if len(sys.argv) != 4:
    print ("Error arguments count. Exiting.")
    sys.exit(-1)

with open(sys.argv[1] + "/" + sys.argv[2], "r") as json_file:
    conf = json.load(json_file)

h_file = open(sys.argv[3] + '/config_defs.h', 'w')

h_file.write("#include \"app_config.h\"\n\n")

#if conf["wifi"] == True:
#    
def isStdTopic(topic):
    if topic.get("std_wifi") == True:
        return True
    if topic.get("std_mqtt") == True:
        return True
    if topic.get("std_ble_mesh") == True:
        return True
    return False

for topic in conf["topics"]:
    if not isStdTopic(topic):
        for elt in topic["elements"]:
            if elt["type"] == "boolean":
                h_file.write("static bool\t" + elt["short_name"] + ";\n")
            if elt["type"] == "array":
                h_file.write("static char\t" + elt["short_name"] + "[" + str(elt["size"]) + "];\n")
            if elt["type"] == "int8":
                h_file.write("static uint8_t\t" + elt["short_name"] + ";\n")
            if elt["type"] == "int16":
                h_file.write("static uint16_t\t" + elt["short_name"] + ";\n")
            if elt["type"] == "int32":
                h_file.write("static uint32_t\t" + elt["short_name"] + ";\n")

for topic in conf["topics"]:
    if topic.get("std_wifi") == True:
        h_file.write("\n#define\t\tAPP_CONFIG_STD_WIFI\t1\n")
        h_file.write("APP_CONFIG_DEFINE_STD_WIFI(\"" + topic["name"] + "\", \"" + topic["default_ssid"] + "\", \"" + topic["default_psk"] + "\")\n")
    elif topic.get("std_mqtt") == True:
        h_file.write("\n#define\t\tAPP_CONFIG_STD_MQTT\t1\n")
        h_file.write("APP_CONFIG_DEFINE_STD_MQTT(\"" + topic["name"] + "\", 1883)\n")
    elif topic.get("std_ble_mesh") == True:
        h_file.write("\n#define\t\tAPP_CONFIG_STD_BLE_MESH\t1\n")
        #h_file.write("APP_CONFIG_DEFINE_STD_WIFI(\"" + topic["name"] + "\", \"" + topic["default_ssid"] + "\", \"" + topic["default_psk"] + "\")\n")
    else:
        h_file.write("\nstatic app_config_element_t config_" + topic["short_name"] + "_elements[] = {\n")
        for elt in topic["elements"]:
            if elt["type"] == "boolean":
                h_file.write("\tAPP_CONFIG_DEFINE_BOOL(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
            if elt["type"] == "array":
                h_file.write("\tAPP_CONFIG_DEFINE_ARRAY(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
            if elt["type"] == "int8":
                h_file.write("\tAPP_CONFIG_DEFINE_INT8(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
            if elt["type"] == "int16":
                h_file.write("\tAPP_CONFIG_DEFINE_INT8(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
            if elt["type"] == "int32":
                h_file.write("\tAPP_CONFIG_DEFINE_INT8(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
        h_file.write("};\n\n")

h_file.write("static app_config_topic_t conf_topics[] = {\n")
for topic in conf["topics"]:
    if topic.get("std_wifi") == True:
        h_file.write("\tAPP_CONFIG_DEFINE_TOPIC(std_wifi_topic , \"" + topic["name"] + "\", config_std_wifi_elements),\n")
    elif topic.get("std_mqtt") == True:
        h_file.write("\tAPP_CONFIG_DEFINE_TOPIC(std_mqtt_topic , \"" + topic["name"] + "\", config_std_mqtt_elements),\n")
    elif topic.get("std_mqtt") == True:
        print("TODO: MQTT topic")
    elif topic.get("std_ble_mesh") == True:
        print("TODO: Mesh topic")
    else:
        h_file.write("\tAPP_CONFIG_DEFINE_TOPIC(" + topic["short_name"] + ", \"" + topic["name"] + "\", config_" + topic["short_name"] + "_elements),\n")
h_file.write("};\n\n")

h_file.write("static app_config_t app_conf = { " + str(conf["version"]) + ", \"" + conf["name"] + "\", \"" + conf["short_name"] + "\", " + str(len(conf["topics"])) + ", conf_topics };")
print(conf["name"])

#html_header = "\nstatic char* app_config_html = \"<html>\n\t<head></head>\n\t<body>\n"
#html_footer = "\t</body>\n</html>\";\n"

#h_file.write(html_header)