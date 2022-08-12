#include "modbus.h"
#include "debug.h"

#include <ModbusMaster.h>

ModbusMaster g_modBus;

constexpr auto g_errNotInitialized = 0xff;

void idleCallback()
{
	yield();
}

bool ModBus::connect()
{
	m_valid = false;

	g_modBus.idle(idleCallback);

	// Try USB first
	Serial.begin(115200);
	delay(500);
    g_modBus.begin(1, Serial);

    auto res = g_modBus.readInputRegisters(55, 2);	// dummy read total energy

    if(res == g_modBus.ku8MBSuccess)
    {
    	m_valid = true;
    	m_baudRate = 115200;
    	return true;
    }

	// Try older serial inverter as a backup if USB failed
    delay(500);
	Serial.begin(9600);
    delay(500);
	g_modBus.begin(1, Serial);
	res = g_modBus.readInputRegisters(28, 2);		// dummy read total energy

    if(res != g_modBus.ku8MBSuccess)
    	return false;

    m_valid = true;
    m_baudRate = 9600;
    return true;
}

int ModBus::readInputRegisters(uint16_t* buffer, uint32_t _address, uint32_t _count)
{
	if(!m_valid)
	{
		if(g_dryRun)
		{
			for(uint16_t i=0; i<_count; ++i)
				buffer[i] = _address + i;
			return ModbusMaster::ku8MBSuccess;
		}
		return g_errNotInitialized;
	}

	const auto res = g_modBus.readInputRegisters(_address, _count);
	if (res != g_modBus.ku8MBSuccess)
		return res;
	for(int i=0; i<_count; ++i)
		buffer[i] = g_modBus.getResponseBuffer(i);
	return res;
}

int ModBus::readHoldingRegisters(uint16_t* buffer, uint32_t _address, uint32_t _count)
{
	if(!m_valid)
	{
		if(g_dryRun)
		{
			for(uint16_t i=0; i<_count; ++i)
				buffer[i] = _address + i;
			return ModbusMaster::ku8MBSuccess;
		}		
		return g_errNotInitialized;
	}

	const auto res = g_modBus.readHoldingRegisters(_address, _count);
	if (res != g_modBus.ku8MBSuccess)
		return res;
	for(int i=0; i<_count; ++i)
		buffer[i] = g_modBus.getResponseBuffer(i);
	return res;
}

int ModBus::writeHoldingRegister(uint16_t _address, uint16_t _value)
{
	if(!m_valid)
		return g_dryRun ? ModbusMaster::ku8MBSuccess : g_errNotInitialized;

	return g_modBus.writeSingleRegister(_address, _value);
}

String ModBus::errorToString(int errorCode)
{
	switch(errorCode)
	{
		case ModbusMaster::ku8MBIllegalFunction:		return "Modbus protocol illegal function exception.";
		case ModbusMaster::ku8MBIllegalDataAddress:	 	return "Modbus protocol illegal data address exception";
		case ModbusMaster::ku8MBIllegalDataValue:		return "Modbus protocol illegal data value exception";
		case ModbusMaster::ku8MBSlaveDeviceFailure:	 	return "Modbus protocol slave device failure exception";

		case ModbusMaster::ku8MBSuccess:				return "ModbusMaster success";
		case ModbusMaster::ku8MBInvalidSlaveID:		 	return "ModbusMaster invalid response slave ID exception";
		case ModbusMaster::ku8MBInvalidFunction:		return "ModbusMaster invalid response function exception";
		case ModbusMaster::ku8MBResponseTimedOut:		return "ModbusMaster response timed out exception";
		case ModbusMaster::ku8MBInvalidCRC:			 	return "ModbusMaster invalid response CRC exception";
		case g_errNotInitialized:					 	return "Connection failed or not initialized";

		default:										return String("Unknown error code ") + String(errorCode);
	}
}
