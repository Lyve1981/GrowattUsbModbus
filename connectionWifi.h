#pragma once

#include "connection.h"

class ConnectionWifi : public Connection
{
public:
	ConnectionWifi() = default;

	virtual bool initialize() override;
	virtual bool isInitialized() override;

	virtual bool connect() override;
	virtual bool isConnected() override;

	virtual Client* getClient() override;
};
