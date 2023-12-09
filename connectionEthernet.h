#pragma once

#include "connection.h"

class ConnectionEthernet : public Connection
{
public:
	ConnectionEthernet() = default;

	virtual bool initialize() override;

	virtual bool connect() override
	{
		return isConnected();
	}

	virtual bool isConnected() override
	{
		return isInitialized();
	}

	virtual Client* getClient() override;

	virtual void loop() override;
};
