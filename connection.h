#pragma once

class Client;

class Connection
{
public:
	virtual bool initialize()
	{
		m_initialized = true;
		return true;
	}
	virtual bool isInitialized()
	{
		return m_initialized;
	}

	virtual bool connect() = 0;
	virtual bool isConnected() = 0;
	
	virtual Client* getClient() = 0;

	virtual void loop() {}

private:
	bool m_initialized = false;
};
