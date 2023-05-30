#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ModbusMaster.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "config.h"
#include "configfile.h"
#include "modbus.h"
#include "debug.h"

constexpr auto LED	= LED_BUILTIN;
constexpr auto ON	= LOW;
constexpr auto OFF	= HIGH;

ConfigFile					g_config;

WiFiManager					g_wm;
WiFiClient					g_wifiClient;
PubSubClient				g_mqttClient(g_wifiClient);
ModBus						g_modbus;
ESP8266WebServer			g_httpServer(80);
ESP8266HTTPUpdateServer		g_httpUpdater;

// runtime config
String		g_mqttServer;
uint32_t	g_mqttPort;
String		g_mqttUser;
String		g_mqttPassword;
String		g_mqttSubTopic;
String		g_mqttPubTopic;
String		g_mqttWillTopic;
String		g_otaUser;
String		g_otaPassword;

char g_jsonOutBuffer[4096];
constexpr uint32_t g_modbusBufferSize = 256;
uint16_t g_modbusBuffer[g_modbusBufferSize];

bool sendBuffer(const char* buffer, uint32_t len)
{
	if(!buffer || !len)
		return false;

	constexpr auto chunkSize = 256;

	g_mqttClient.beginPublish(g_mqttPubTopic.c_str(), (uint32_t)len, false);
	auto remaining = len;
	const uint8_t* buf = (const uint8_t*)buffer;
	while(remaining > 0)
	{
		auto sendSize = remaining > 128 ? 128 : remaining;

		g_mqttClient.write(buf, sendSize);

		remaining -= sendSize;
		buf += sendSize;
	}
	g_mqttClient.endPublish();

	return true;
}

bool sendJson(DynamicJsonDocument& json)
{
	json["timestamp"] = millis();
	json["baud"] = g_modbus.getBaudRate();
	json["rssi"] = WiFi.RSSI();
	json["ssid"] = WiFi.SSID();
	json["ip"] = WiFi.localIP().toString();

	const auto len = serializeJson(json, g_jsonOutBuffer);

	if(len <= 0)
		return false;

	return sendBuffer(g_jsonOutBuffer, len);
}

bool sendError(const StaticJsonDocument<1024>* request, const char* error)
{
	DynamicJsonDocument r(2048);
	r["status"] = "error";
	if(request)
		r["request"] = *request;
	r["error"] = error;
	return sendJson(r);
}

bool sendError(const StaticJsonDocument<1024>& request, const char* error)
{
	return sendError(&request, error);
}
bool sendError(const char* error)
{
	return sendError(nullptr, error);
}

bool readModBusRegisters(const StaticJsonDocument<1024>& request, bool holdingRegs)
{
	const int first = request["index"];
	const int count = request["count"];

	if(first < 0)
		return sendError(request, "Parameter 'index' must be >= 0");
	if(count <= 0)
		return sendError(request, "Parameter 'count' must be > 0");
	if(count > g_modbusBufferSize)
		return sendError(request, (String("Parameter 'count' must be <= ") + String(g_modbusBufferSize)).c_str());

	constexpr auto retryCount = 3;
	int errorCount = 0;

	while(true)
	{
		const auto err = holdingRegs ? 
			g_modbus.readHoldingRegisters(g_modbusBuffer, first, count) : 
			g_modbus.readInputRegisters(g_modbusBuffer, first, count);

		if(!err)
			break;

		++errorCount;

		if(err != ModbusMaster::ku8MBResponseTimedOut || errorCount == retryCount)
			return sendError(request, (String("Failed to read modbus registers, error code ") + String(err) + " - " + g_modbus.errorToString(err)).c_str());

		delay(100);
	}

	DynamicJsonDocument response(5 * 1024);

	response["status"] = "ok";
	response["request"] = request;
	response["retryCount"] = errorCount;

	auto arr = response["data"].to<JsonArray>();

	for(int i=0; i<count; ++i)
		arr.add(g_modbusBuffer[i]);

	return sendJson(response);
}

bool writeModBusHoldingRegister(const StaticJsonDocument<1024>& request)
{
	const int index = request["index"];
	const int value = request["value"];

	if(index < 0)		return sendError(request, "Parameter 'index' must be >= 0");
	if(index > 65535)	return sendError(request, "Parameter 'index' must be <= 65535");
	if(value < 0)		return sendError(request, "Parameter 'value' must be >= 0");
	if(value > 65535)	return sendError(request, "Parameter 'value' must be <= 65535");

	const auto err = g_modbus.writeHoldingRegister(index, value);

	if(err)
		return sendError(request, (String("Failed to write modbus register, error code ") + String(err) + " - " + g_modbus.errorToString(err)).c_str());

	DynamicJsonDocument response(1024);

	response.clear();
	
	response["status"] = "ok";
	response["request"] = request;

	return sendJson(response);
}

void resetSettings()
{
	g_wm.resetSettings();
	delay(1000);
	ESP.restart();	
}

void onMqttMessage(const char* topic, byte* payload, unsigned int length)
{
	payload[length] = 0;

	char* json = (char*)payload;

	debugln("MQTT message: topic = " + String(topic) + ", payload = " + String((char*)payload));

	StaticJsonDocument<1024> doc;
	const auto error = deserializeJson(doc, json);

	if(error)
	{
	    debug("deserializeJson() failed: ");
	    debugln(error.f_str());
	    return;
	}

	String command = doc["command"];

	if(command.length() == 0)
		return;

	command.toLowerCase();

	if(command == "reboot")
	{
		ESP.restart();
		return;
	}

	if(command == "resetsettings")
	{
		resetSettings();
		return;
	}

	if(command == "readinputregisters")
	{
		readModBusRegisters(doc, false);
		return;
	}
	if(command == "readholdingregisters")
	{
		readModBusRegisters(doc, true);
		return;
	}
	if(command == "writeholdingregister")
	{
		writeModBusHoldingRegister(doc);
		return;
	}

	sendError(doc, "Unknown request");
}

bool wifiReconnect(bool forceConfigPortal = false)
{
    if (WiFi.status() == WL_CONNECTED)
		return true;

	digitalWrite(LED, ON);

	if(forceConfigPortal && !g_wm.startConfigPortal("GrowattUSB", "growattusb"))
		return false;
	if(!forceConfigPortal && !g_wm.autoConnect("GrowattUSB", "growattusb"))
		return false;

    while (WiFi.status() != WL_CONNECTED)
    {
		delay(200);
		digitalWrite(LED, !digitalRead(LED));
    }

	digitalWrite(LED, OFF);

	return true;
}

bool mqttReconnect()
{
    if (g_mqttServer.length() == 0)
        return false;

    if (WiFi.status() != WL_CONNECTED)
        return false;

    if (g_mqttClient.connected())
        return true;

	debugln((String("MQTT not connected, state = ") + String(g_mqttClient.state())).c_str());

	const auto t = millis();

	while(true)
	{
		g_mqttClient.setServer(g_mqttServer.c_str(), g_mqttPort);

		const auto mqttClientName = String("esp8266_") + String(ESP.getChipId());

		if(g_mqttClient.connect(mqttClientName.c_str(), g_mqttUser.c_str(), g_mqttPassword.c_str(), g_mqttWillTopic.c_str(), 2, true, "offline"))
		{
			debugln("MQTT connection established");
			g_mqttClient.setCallback(onMqttMessage);
			g_mqttClient.subscribe(g_mqttSubTopic.c_str());
			g_mqttClient.publish(g_mqttWillTopic.c_str(), "online", true);
			return true;
		}
		else
		{
			debugln("MQTT connect failed");
			if(millis() - t > 20000)
			{
				WiFi.disconnect();
				wifiReconnect(true);
				delay(1000);
				ESP.restart();
				break;
			}
			else
			{
				digitalWrite(LED, !digitalRead(LED));
				delay(1500);
			}
		}
	}

	return false;
}

bool modbusReconnect()
{
	if(g_dryRun)
		return true;

	if(g_modbus.isValid())
		return true;

	digitalWrite(LED, OFF);
	
	const auto t = millis();

	auto myDelay = [&](uint32_t ms)
	{
		const auto t = millis();
		while((millis() - t) < ms)
		{
			yield();
			g_mqttClient.loop();
			g_httpServer.handleClient();
		}
	};

	while((millis() - t) < 20000)
	{
		if(g_modbus.connect())
			return true;

		digitalWrite(LED, ON);
		myDelay(500);

		digitalWrite(LED, OFF);
		myDelay(500);
	}
	
	sendError("Modbus connection failed");

	// give us some time to report the modbus connection error via MQTT
	{
		const auto t = millis();
		while((millis() - t) < 3000)
			g_mqttClient.loop();
	}

	return false;
}

bool reconnectAll()
{
	if(!wifiReconnect())		return false;
	if(!mqttReconnect())		return false;
	if(!modbusReconnect())		return false;

	return true;
}

void setup()
{
	LittleFS.begin();

	if(g_debug)
		Serial.begin(115200);	// debug

	pinMode(LED, OUTPUT);

    WiFi.hostname(g_config.get("hostname", DefaultConfig::hostName));
    WiFi.mode(WIFI_STA);

    delay(2000);

	g_mqttServer = g_config.get("g_mqttServer", DefaultConfig::mqttServer);
	g_mqttPort = g_config.get("g_mqttPort", String(DefaultConfig::mqttPort).c_str()).toInt();
	g_mqttUser = g_config.get("g_mqttUser", DefaultConfig::mqttUser);
	g_mqttPassword = g_config.get("g_mqttPassword", DefaultConfig::mqttPassword);
	g_mqttSubTopic = g_config.get("g_mqttSubTopic", DefaultConfig::mqttSubTopic);
	g_mqttPubTopic = g_config.get("g_mqttPubTopic", DefaultConfig::mqttPubTopic);
	g_mqttWillTopic = g_config.get("g_mqttWillTopic", DefaultConfig::mqttWillTopic);
	g_otaUser = g_config.get("g_otaUser", DefaultConfig::otaUser);
	g_otaPassword = g_config.get("g_otaPassword", DefaultConfig::otaPassword);

    auto mqttServer = new WiFiManagerParameter("server", "MQTT Server", g_mqttServer.c_str(), 32);
    auto mqttPort = new WiFiManagerParameter("port", "MQTT Port", String(g_mqttPort).c_str(), 5);
    auto mqttUser = new WiFiManagerParameter("user", "MQTT Username", g_mqttUser.c_str(), 32);
    auto mqttPassword = new WiFiManagerParameter("pass", "MQTT Password", g_mqttPassword.c_str(), 32);
    auto mqttTopicSub = new WiFiManagerParameter("topicSub", "MQTT Command Topic", g_mqttSubTopic.c_str(), 64);
    auto mqttTopicPub = new WiFiManagerParameter("topicPub", "MQTT Publish Topic", g_mqttPubTopic.c_str(), 64);
    auto mqttTopicWill = new WiFiManagerParameter("topicWill", "MQTT Will Topic", g_mqttWillTopic.c_str(), 64);
    auto otaUser = new WiFiManagerParameter("otaUser", "OTA Update User", g_otaUser.c_str(), 64);
    auto otaPassword = new WiFiManagerParameter("otaPassword", "OTA Update Password", g_otaPassword.c_str(), 64);

    g_wm.addParameter(mqttServer);
    g_wm.addParameter(mqttPort);
    g_wm.addParameter(mqttUser);
    g_wm.addParameter(mqttPassword);
    g_wm.addParameter(mqttTopicSub);
    g_wm.addParameter(mqttTopicPub);
    g_wm.addParameter(mqttTopicWill);
    g_wm.addParameter(otaUser);
    g_wm.addParameter(otaPassword);

    g_wm.setSaveParamsCallback([&]()
    {
    	g_mqttServer = mqttServer->getValue();
    	g_mqttPort = String(mqttPort->getValue()).toInt();
    	g_mqttUser = mqttUser->getValue();
    	g_mqttPassword = mqttPassword->getValue();
    	g_mqttSubTopic = mqttTopicSub->getValue();
    	g_mqttPubTopic = mqttTopicPub->getValue();
    	g_mqttWillTopic = mqttTopicWill->getValue();
    	g_otaUser = otaUser->getValue();
    	g_otaPassword = otaPassword->getValue();

		g_config.set("g_mqttServer", g_mqttServer.c_str());
		g_config.set("g_mqttPort", String(g_mqttPort).c_str());
		g_config.set("g_mqttUser", g_mqttUser.c_str());
		g_config.set("g_mqttPassword", g_mqttPassword.c_str());
		g_config.set("g_mqttSubTopic", g_mqttSubTopic.c_str());
		g_config.set("g_mqttPubTopic", g_mqttPubTopic.c_str());
		g_config.set("g_mqttWillTopic", g_mqttWillTopic.c_str());
		g_config.set("g_otaUser", g_otaUser.c_str());
		g_config.set("g_otaPassword", g_otaPassword.c_str());

		delay(1000);
    });

    g_wm.setConfigPortalTimeout(60);

	if(!reconnectAll())
		ESP.restart();

    g_httpUpdater.setup(&g_httpServer, "/update", g_otaUser, g_otaPassword);
    g_httpServer.begin();

	debugln("Boot completed");
}

void loop()
{
	if(!reconnectAll())
		return;

	g_mqttClient.loop();
	g_httpServer.handleClient();

	auto t = millis() & 0x1ff;

	if(t < 50)
		digitalWrite(LED, ON);
	else
		digitalWrite(LED, OFF);
}
