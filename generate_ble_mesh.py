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

# Open files
with open(json_filename, "r") as json_file:
    conf = json.load(json_file)

h_file = open(h_filename, 'w')

output = ""

h_file.write(output)