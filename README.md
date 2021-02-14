# RGB Led strip controller

This project is an RGB led strip controller powered by an ESP8266 (NodeMCU) and uses a Light-dependent Resistor (LDR) to turn on and of the strip depending on room luminosity.

Color, light trigger value and operation mode is full controlled via HTTP requests

Power in is 12V DC.

To control the strip via HTTP you must set your SSID and PASSWORD in the controller code.

## Materials
* NodeMCU (ESP8266)
* 12V RGB Led strip
* 12V power adapter
* L7805V Voltage regulator
* ULN2003A IC
* LDR module or LDR resistor + 10kΩ resistor

## PCB

Provided [fritzing](https://fritzing.org/) PCB Schema
![PCB](pcb.png?raw=true "PCB")

## Endpoints

### /on
Set mode to always on

### /off
Set mode to always off

### /auto
Set mode to always auto (on if light is less than `lightTriggerValue`). Mode by default

### /color?color=[rrggbb]
Set strip color to `color`. color is and `rrggbb` hex value without `#`. By default is `ffffff`


### /light-trigger?value=[number]
Set the light trigger value, by default is `900`

### /status
Return status

All endpoints return status after execute the command

`{ mode: "off", status: true, color: "ffffff", sensor-value: 1024, sensor-status: false, light-trigger-value: 900 }`