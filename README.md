## Growatt USB ModBus to MQTT proxy

# Description

This project is a custom data logger for Growatt inverters with an USB connector.

Usually, you connect a ShineWifi-X or ShineLan-X connector (sold by Growatt) to the USB connector of your inverter to monitor your data, but you are restricted to the Growatt Cloud, aka ShineServer. This project replaces the need for a Shine* module by exposing the ModBus interface via MQTT.

Benefits:

- Works without internet / no cloud connection needed
- Access more data than is usually exposed via ShineServer
- Faster update speed. If you want, you can even read the data once every second
- Further process your data / integrate into your Smart Home

# Installation & Usage

Installation & Usage instructions [can be found in the Wiki](https://github.com/Lyve1981/GrowattUsbModbus/wiki).
