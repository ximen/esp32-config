#!/usr/bin/python3

import json
import sys

if len(sys.argv) != 4:
    print ("Error arguments count. Exiting.")
    sys.exit(-1)

with open(sys.argv[1] + "/" + sys.argv[2], "r") as json_file:
    conf = json.load(json_file)

h_file = open(sys.argv[3] + '/config_defs.h', 'w')

h_file.write("#include \"app_config.h\"\n")
h_file.write("#include \"sdkconfig.h\"\n")
h_file.write("#include <stdbool.h>\n\n")

#if conf["wifi"] == True:
#    
def isStdTopic(topic):
    if topic.get("std_wifi") == True:
        return True
    if topic.get("std_mqtt") == True:
        return True
    return False

for topic in conf["topics"]:
    if not isStdTopic(topic):
        for elt in topic["elements"]:
            if elt["type"] == "boolean":
                if elt.get("default"):
                    h_file.write("static bool\t" + elt["short_name"] + " = true;\n")
                else:
                    h_file.write("static bool\t" + elt["short_name"] + ";\n")
            if elt["type"] == "array":
                h_file.write("static char\t" + elt["short_name"] + "[" + str(elt["size"]) + "];\n")
            if elt["type"] == "string":
                if elt.get("default"):
                    h_file.write("static char\t" + elt["short_name"] + "[" + str(elt["size"]) + "] = \"{0}\";\n".format(elt["default"]))
                else:
                    h_file.write("static char\t" + elt["short_name"] + "[" + str(elt["size"]) + "];\n")
            if elt["type"] == "int8":
                if elt.get("default"):
                    h_file.write("static uint8_t\t {0} = {1};\n".format(elt["short_name"], elt["default"]));
                else:
                    h_file.write("static uint8_t\t" + elt["short_name"] + ";\n")
            if elt["type"] == "int16":
                if elt.get("default"):
                    h_file.write("static uint16_t\t {0} = {1};\n".format(elt["short_name"], elt["default"]));
                else:
                    h_file.write("static uint16_t\t" + elt["short_name"] + ";\n")
            if elt["type"] == "int32":
                if elt.get("default"):
                    h_file.write("static uint32_t\t {0} = {1};\n".format(elt["short_name"], elt["default"]));
                else:
                    h_file.write("static uint32_t\t" + elt["short_name"] + ";\n")

for topic in conf["topics"]:
    if topic.get("std_wifi") == True:
        h_file.write("\n#define\t\tAPP_CONFIG_STD_WIFI\t1\n")
        h_file.write("APP_CONFIG_DEFINE_STD_WIFI(\"" + topic["name"] + "\", \"" + topic["default_ssid"] + "\", \"" + topic["default_psk"] + "\")\n")
    elif topic.get("std_mqtt") == True:
        h_file.write("\n#define\t\tAPP_CONFIG_STD_MQTT\t1\n")
        if topic.get("discovery"):
            h_file.write("APP_CONFIG_DEFINE_STD_MQTT(\"" + topic["name"] + "\", 1883, true)\n")
        else:
            h_file.write("APP_CONFIG_DEFINE_STD_MQTT(\"" + topic["name"] + "\", 1883, false)\n")
    else:
        h_file.write("\nstatic app_config_element_t config_" + topic["short_name"] + "_elements[] = {\n")
        for elt in topic["elements"]:
            if elt["type"] == "boolean":
                h_file.write("\tAPP_CONFIG_DEFINE_BOOL(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
            if elt["type"] == "array":
                h_file.write("\tAPP_CONFIG_DEFINE_ARRAY(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
            if elt["type"] == "string":
                h_file.write("\tAPP_CONFIG_DEFINE_STRING(" + elt["short_name"] + ", \"" + elt["name"] + "\"),\n")
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
    else:
        h_file.write("\tAPP_CONFIG_DEFINE_TOPIC(" + topic["short_name"] + ", \"" + topic["name"] + "\", config_" + topic["short_name"] + "_elements),\n")
h_file.write("};\n\n")

h_file.write("static app_config_t app_conf = { " + str(conf["version"]) + ", \"" + conf["name"] + "\", \"" + conf["short_name"] + "\", " + str(len(conf["topics"])) + ", conf_topics };")
print(conf["name"])
