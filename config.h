#pragma once

namespace DefaultConfig
{
	constexpr const char* const hostName = "GrowattMB2USB";
	constexpr const char* const wifiPassword = "growatt";
	
	// MQTT
	constexpr const char* const mqttServer = "192.168.20.10";
	constexpr const int32_t mqttPort = 1883;
	constexpr const char* const mqttUser = "";
	constexpr const char* const mqttPassword = "";
	constexpr const char* const mqttSubTopic = "energy/gromodbus/request";
	constexpr const char* const mqttPubTopic = "energy/gromodbus/response";
	constexpr const char* const mqttWillTopic = "energy/gromodbus/status";

	// Over-the-Air updater
	constexpr const char* const otaUser = "ota";
	constexpr const char* const otaPassword = "growatt";
}
