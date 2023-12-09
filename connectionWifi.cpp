#include "connectionWifi.h"

#include <ESP8266WiFi.h>
#include "WifiClient.h"

bool ConnectionWifi::initialize()
{
	return Connection::initialize();
}

bool ConnectionWifi::isInitialized()
{
	return Connection::isInitialized();
}

bool ConnectionWifi::connect()
{
	return false;
}

bool ConnectionWifi::isConnected()
{
	return WiFi.status() == WL_CONNECTED;
}

Client* ConnectionWifi::getClient()
{
	return new WiFiClient();
}
