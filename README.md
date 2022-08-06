## Growatt USB ModBus to MQTT proxy

# Description

This project is a custom data logger for Growatt inverters with an USB connector.

Usually, you connect a ShineWifi-X or ShineLan-X connector (sold by Growatt) to the USB connector of your inverter to monitor your data, but you are restricted to the Growatt Cloud, aka ShineServer. This project replaces the need for a Shine* module by exposing the ModBus interface via MQTT.

Benefits:

- Works without internet / no cloud connection needed
- Access more data than is usually exposed via ShineServer
- Faster update speed. If you want, you can even read the data once every second
- Further process your data / integrate into your Smart Home

# USB connection

This project is heavily inspired by the ShineWifi-S firmwware replacement project: https://github.com/otti/Growatt_ShineWiFi-S

Thanks a lot for giving **the** important hint that the USB connector is just a wrapper for RS232

What the Growatt Inverter wants to see on the USB port is a CH340 USB => Serial converter so this project is compatible with ESP devices that have such a serial interface, for example the NodeMCU Lolin V3, Wemos D1, etc

# Installation

- Grab any ESP board with an CH340 USB <=> serial chip
- Connect the ESP board to your computer via USB
- Open this project in Arduino IDE and upload to your ESP board
- Connect the ESP board to your Growatt inverter
- Connect to the Wifi AP named ``GrowattUSB`` and adjust your Wifi Settings and MQTT settings
- Wait until the LED flashes about two times per second.

# Usage

As there are too many different inverters with varying ModBus registers, I kept this project generic. You can request register values via MQTT, the response will be a register => value map as json. Mapping these registers to useful values is on your own. I use Node-RED to do this, I'm gonna commit the Node-RED flow soon.

The ESP communicates via MQTT in json format. Once you've installed everything, you can send an MQTT message to request a set of ModBus register from your Growatt inverter. Example:
```
{
   "command": "readinputregisters",
   "index": 1000,
   "count": 125
}
```

The ESP will respond with a message such as this upon success:
```
{
  "status": "ok",
  "request": {
    "command": "readinputregisters",
    "index": 100,
    "count": 25
  },
  "data": [
    100,
    101,
    102,
    ......
  ],
  "timestamp": 1151448
}
```

In case of an error, an error response is returned:
```
{
   "status":"error",
   "request":{
      "command":"readinputregisters",
      "index":3093,
      "count":3
   },
   "error":"Failed to read modbus registers, error code 1 - Modbus protocol illegal function exception.",
   "timestamp":1227115
}
```

# Compatibility

This is a new project, it has been tested only with a Growatt SPH-5000TL3 and is actively being used. If you use this project with a different inverter, please report back how it works for you.
