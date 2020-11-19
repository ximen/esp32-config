#!/usr/bin/python3

import sys
import json
import os.path
from jsmin import jsmin
from cssmin import cssmin
import htmlmin
import string
from configparser import ConfigParser

sdkconfig = ConfigParser()
with open(sys.argv[1] + "/sdkconfig") as stream:
    sdkconfig.read_string("[sdk]\n" + stream.read())

json_filename = sys.argv[1] + "/" + sdkconfig['sdk']['CONFIG_APP_CONFIG_FILE_NAME'].strip('"')  # Configurations JSON file
css_filename = sys.argv[1] + "/" + sdkconfig['sdk']['CONFIG_APP_CONFIG_CSS_FILE'].strip('"')    # User CSS file
default_css_filename = sys.argv[2] + "/" + 'config_default.css'                                 # Default CSS file
html_filename = sys.argv[1] + "/" + 'config.html'                                               # Output HTML file
h_filename = sys.argv[2] + "/" + 'config_html.h'                                                # Output HTML include file

if sdkconfig['sdk']['CONFIG_APP_CONFIG_MINIMIZE_HTML'] == "y":
    minimize = True
else:
    minimize = False

def xxd():
    with open(html_filename, 'r') as f:
        output = "char app_config_html[] = {"
        length = 0
        while True:
            buf = f.read(12)

            if not buf:
                output = output[:-2]
                break
            else:
                output += "\n  "

            for i in buf:
                output += "0x%02x, " % ord(i)
                length += 1
        output += "\n};\n"
        output += "unsigned int app_config_html_len = %d;" % length
        return output


# Determines if topic is standard
def isStdTopic(topic):
    if topic.get("std_wifi") == True:
        return True
    if topic.get("std_ble_mesh") == True:
        return True
    if topic.get("std_mqtt") == True:
        return True
    return False

def get_short_name(topic):
    if not isStdTopic(topic):
        return topic["short_name"]
    else:
        if topic.get("std_wifi") == True:
            return "std_wifi_topic"
        if topic.get("std_ble_mesh") == True:
            return "std_ble_mesh_topic"
        if topic.get("std_mqtt") == True:
            return "std_mqtt_topic"

# Open files
with open(json_filename, "r") as json_file:
    conf = json.load(json_file)

html_file = open(html_filename, 'w')

if not os.path.exists(css_filename):
    css_filename = default_css_filename

with open(css_filename, 'r') as css_file:
    css = css_file.read()

# JS script for tabs
tabs_script = '''function openTab(evt, cityName) {
  var i, tabcontent, tablinks;

  tabcontent = document.getElementsByClassName("tabcontent");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }

  tablinks = document.getElementsByClassName("tablinks");
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].className = tablinks[i].className.replace(" active", "");
  }

  document.getElementById(cityName).style.display = "block";
  evt.currentTarget.className += " active";
}

document.getElementById("defaultOpen").click();

'''
deps_script = '''var dependencies={
'''

attach_script = '''for (const [key, value] of Object.entries(dependencies)) {
    var elt = document.getElementsByName(value)[0];
    elt.onclick = function(){
        if (this.checked) {
            for (const [key, value] of Object.entries(dependencies)) {
                if (value == this.name)
                document.getElementsByName(key)[0].style.display = "block";
            }
        } else {
            for (const [key, value] of Object.entries(dependencies)) {
                if (value == this.name)
                document.getElementsByName(key)[0].style.display = "none";
            }
        }
    }
    elt.onclick();
}
'''
action_script = '''
function fillValues(config){{
    for (topic of config.topics){{
        for (elt of topic.elements){{
            input = document.getElementsByName(elt.short_name)[0]
            if (elt.type == "bool"){{
                input.checked = elt.value;
                if(input.onclick) input.onclick();
            }} else if (elt.type == "int") {{
                input.value = elt.value;
            }} else if (elt.type == "array") {{
                input.value = elt.value;
            }} else if (elt.type == "string") {{
                input.value = elt.value;
            }}
        }}
    }}
}}
var url = window.location.protocol + "//" + window.location.hostname + {}
fetch(url)
  .then(response => response.json())
  .then(json => fillValues(json));
'''.format(sdkconfig['sdk']['CONFIG_APP_CONFIG_REST_PATH'])

if minimize:
    css = cssmin(css)
# Write standard HTML header
html = '<html>\n\t<head>\n\t\t<meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">\n\t\t<title>{}</title>\n\t\t<style>{}\n\t\t</style>\n\t</head>\n\t<body style="width: 90%; max-width: 1024px;margin-right: auto;margin-left: auto;background-color:#E6E6FA">\n'.format(conf["name"], css)
html += '\t\t<h1>{}</h1>\n'.format(conf["name"])

html += '<form action="{}" method="post">\n'.format(sdkconfig['sdk']['CONFIG_APP_CONFIG_HTML_SET_PATH'].strip('"'))
# Generate tabs with topics
# Tabs
counter = 0
html += '\t\t<div class=\"tab\">\n'
for topic in conf["topics"]:
    if counter == 0:
        default_open = 'id=\"defaultOpen\"'
    else: 
        default_open = ''
    html += '\t\t\t<button type="button" class=\"tablinks\" name={2} onclick=\"openTab(event, \'{0}\')\" {1}>{0}</button>\n'.format(topic["name"], default_open, get_short_name(topic))
    counter += 1
html += '\t\t</div>\n'

# Fill dependencies
for topic in conf["topics"]:
    if topic.get("depends_on"):
        deps_script += '"{0}": "{1}",\n'.format(get_short_name(topic), topic["depends_on"])
    if not isStdTopic(topic):
        for elt in topic["elements"]:
            if elt.get("depends_on"):
                deps_script += '"{0}": "{1}",\n'.format(elt["short_name"], elt["depends_on"])
deps_script += "}\n"

# Elements inside tabs
for topic in conf["topics"]:
    html += '\t\t<div id=\"{}\" class=\"tabcontent\">\n'.format(topic["name"])
    if not isStdTopic(topic):
        html += '<fieldset class="table">\n'
        for elt in topic["elements"]:
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">{0}</div>\n'.format(elt["name"])
            if elt["type"] == "boolean":
                html += '\t\t\t<div class="td"><input type="checkbox" name="{0}"></div>\n'.format(elt["short_name"])
            if elt["type"] == "array":
                html += '\t\t\t<div class="td"><input type="text" maxlength="{0}" name="{1}"></div>\n'.format(elt["size"], elt["short_name"])
            if elt["type"] == "string":
                html += '\t\t\t<div class="td"><input type="text" maxlength="{0}" name="{1}"></div>\n'.format(elt["size"], elt["short_name"])
            if elt["type"] == "int8":
                html += '\t\t\t<div class="td"><input type="number" max="255" name="{0}"></div>\n'.format(elt["short_name"])
            if elt["type"] == "int16":
                html += '\t\t\t<div class="td"><input type="number" max="65535" name="{0}"></div>\n'.format(elt["short_name"])
            if elt["type"] == "int32":
                html += '\t\t\t<div class="td"><input type="number" name="{0}"></div>\n'.format(elt["short_name"])
            html += '</div>\n'
        html += '</fieldset>\n'
    else:
        if topic.get("std_wifi") == True:
            # Standard WiFi fields
            html += '<fieldset class="table"><legend>Wi-Fi settings <abbr title="This field is mandatory">*</abbr></legend>\n'
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">Access point</div>\n'
            html += '\t\t\t<div class="td"><input type="checkbox" name="std_wifi_ap"></div>\n'
            html += '</div>\n'
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">SSID</div>\n'
            html += '\t\t\t<div class="td"><input type="text" maxlength="32" name="std_wifi_ssid"></div>\n'
            html += '</div>\n'
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">PSK</div>\n'
            html += '\t\t\t<div class="td"><input type="password" maxlength="64" name="std_wifi_psk"></div>\n'
            html += '</div>\n'
            html += '</fieldset>\n'
        elif topic.get("std_mqtt") == True:
            html += '<fieldset class="table"><legend>MQTT <abbr title="This field is mandatory">*</abbr></legend>\n'
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">Broker</div>\n'
            html += '\t\t\t<div class="td"><input type="text" name="std_mqtt_broker"></div>\n'
            html += '</div>\n'
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">Port</div>\n'
            html += '\t\t\t<div class="td"><input type="number" name="std_mqtt_port"></div>\n'
            html += '</div>\n'
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">Username</div>\n'
            html += '\t\t\t<div class="td"><input type="text" name="std_mqtt_user"></div>\n'
            html += '</div>\n'
            html += '<div class="tr">\n'
            html += '\t\t\t<div class="td right">Password</div>\n'
            html += '\t\t\t<div class="td"><input type="password" name="std_mqtt_pass"></div>\n'
            html += '</div>\n'
            html += '</fieldset>\n'
        elif topic.get("std_ble_mesh") == True:
            html += '<fieldset class="table"><legend>Bluetooth Mesh settings<abbr title="This field is mandatory">*</abbr></legend>\n'
            html += '</fieldset>\n'
        else:
            html += '<fieldset class="table">\n'
            html += '</fieldset>\n'
    html += '\t\t</div>\n'

html += '<p align="right"><input type="submit" value="Submit"></p>\n</form>\n'
js = tabs_script + deps_script + attach_script + action_script
if minimize:
    html = htmlmin.minify(html, remove_comments=True, reduce_boolean_attributes=True)
    js = jsmin(js)
    
# Script
html += '\t\t<script>\n{}\n\t\t\n</script>'.format(js)
# Write footer
html += '\t</body>\n</html>'
html_file.write(html)
html_file.close()
h_file = open(h_filename, 'w')
h_file.write(xxd())