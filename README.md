## Growatt USB ModBus to MQTT converter

# Description

This project for Arduino IDE is intended for newer Growatt Inverters with an USB connector. Usually, you connect a ShineWifi-X or ShineLan-X module by Growatt, but you are restricted to the Growatt Cloud.

This project aims at replacing the need for a Shine* module by exposing the ModBus interface via MQTT.

What the Growatt Inverter wants to see on the USB port is a CH340 USB => Serial converter so this project is compatible with ESP devices that have such a serial interface, for example NodeMCU V3, Wemos D1, etc

As there are too many different inverters with varying ModBus registers, I kept this project generic. You can request register values via MQTT, the response will be a register => value map as json. Mapping these registers to useful values is on your own.

# State

Work-in-progress, untested. I'm working on it 😉 But I thought I give you a description what this does beforehand.
