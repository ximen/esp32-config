#!/usr/bin/python3

import sys
import json
import os.path

json_filename = sys.argv[1] + "/" + sys.argv[2]                     # Configurations JSON file
css_filename = sys.argv[1] + "/" + sys.argv[3]                      # User CSS file
default_css_filename = sys.argv[4] + "/" + 'config_default.css'     # Default CSS file
html_filename = sys.argv[1] + "/" + 'config.html'                   # Output HTML file

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

h_file = open(html_filename, 'w')

if not os.path.exists(css_filename):
    css_filename = default_css_filename

with open(css_filename, 'r') as css_file:
    css = css_file.read()

# JS script
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

# Write standard HTML header
h_file.write('<html>\n\t<head>\n\t\t<meta charset=\"utf-8\">\n\t\t<title>{}</title>\n\t\t<style>{}\n\t\t</style>\n\t</head>\n\t<body style="width: 90%; max-width: 1024px;margin-right: auto;margin-left: auto;background-color:#E6E6FA">\n'.format(conf["name"], css))
h_file.write('\t\t<h1>{}</h1>\n'.format(conf["name"]))

h_file.write('<form action="/set" method="get">\n')
# Generate tabs with topics
# Tabs
counter = 0
h_file.write('\t\t<div class=\"tab\">\n')
for topic in conf["topics"]:
    if counter == 0:
        default_open = 'id=\"defaultOpen\"'
    else: 
        default_open = ''
    h_file.write('\t\t\t<button type="button" class=\"tablinks\" name={2} onclick=\"openTab(event, \'{0}\')\" {1}>{0}</button>\n'.format(topic["name"], default_open, get_short_name(topic)))
    counter += 1
h_file.write('\t\t</div>\n')

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
    h_file.write('\t\t<div id=\"{}\" class=\"tabcontent\">\n'.format(topic["name"]))
    if not isStdTopic(topic):
        h_file.write('<fieldset class="table">\n')
        for elt in topic["elements"]:
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">{0}</div>\n'.format(elt["name"]))
            if elt["type"] == "boolean":
                h_file.write('\t\t\t<div class="td"><input type="checkbox" name="{0}" value="**{0}"></div>\n'.format(elt["short_name"]))
            if elt["type"] == "array":
                h_file.write('\t\t\t<div class="td"><input type="text" maxlength="{0}" name="{1}"></div>\n'.format(elt["size"], elt["short_name"]))
            if elt["type"] == "int8":
                h_file.write('\t\t\t<div class="td"><input type="number" max="255" name="{0}"></div>\n'.format(elt["short_name"]))
            if elt["type"] == "int16":
                h_file.write('\t\t\t<div class="td"><input type="number" max="65535" name="{0}"></div>\n'.format(elt["short_name"]))
            if elt["type"] == "int32":
                h_file.write('\t\t\t<div class="td"><input type="number" name="{0}"></div>\n'.format(elt["short_name"]))
            h_file.write('</div>\n')
        h_file.write('</fieldset>\n')
    else:
        if topic.get("std_wifi") == True:
            # Standard WiFi fields
            h_file.write('<fieldset class="table"><legend>Wi-Fi settings <abbr title="This field is mandatory">*</abbr></legend>\n')
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">Access point</div>\n')
            h_file.write('\t\t\t<div class="td"><input type="checkbox" name="std_wifi_ap" value="**std_wifi_ap"></div>\n')
            h_file.write('</div>\n')
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">SSID</div>\n')
            h_file.write('\t\t\t<div class="td"><input type="text" maxlength="32" name="std_wifi_ssid" required></div>\n')
            h_file.write('</div>\n')
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">PSK</div>\n')
            h_file.write('\t\t\t<div class="td"><input type="password" maxlength="64" name="std_wifi_psk" required></div>\n')
            h_file.write('</div>\n')
            h_file.write('</fieldset>\n')
        elif topic.get("std_mqtt") == True:
            h_file.write('<fieldset class="table"><legend>MQTT <abbr title="This field is mandatory">*</abbr></legend>\n')
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">Broker</div>\n')
            h_file.write('\t\t\t<div class="td"><input type="text" name="std_mqtt_broker" required></div>\n')
            h_file.write('</div>\n')
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">Port</div>\n')
            h_file.write('\t\t\t<div class="td"><input type="number" name="std_mqtt_port"></div>\n')
            h_file.write('</div>\n')
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">Username</div>\n')
            h_file.write('\t\t\t<div class="td"><input type="text" name="std_mqtt_user"></div>\n')
            h_file.write('</div>\n')
            h_file.write('<div class="tr">\n')
            h_file.write('\t\t\t<div class="td right">Password</div>\n')
            h_file.write('\t\t\t<div class="td"><input type="text" name="std_mqtt_pass"></div>\n')
            h_file.write('</div>\n')
            h_file.write('</fieldset>\n')
        elif topic.get("std_ble_mesh") == True:
            h_file.write('<fieldset class="table"><legend>Bluetooth Mesh settings<abbr title="This field is mandatory">*</abbr></legend>\n')
            h_file.write('</fieldset>\n')
        else:
            h_file.write('<fieldset class="table">\n')
            h_file.write('</fieldset>\n')
    h_file.write('\t\t</div>\n')

h_file.write('<p align="right"><input type="submit" value="Submit"></p>\n</form>\n')
# Script
h_file.write('\t\t<script>\n{0}{1}{2}\n\t\t\n</script>'.format(tabs_script, deps_script, attach_script))
# Write footer
h_file.write('\t</body>\n</html>')

print(conf["name"])