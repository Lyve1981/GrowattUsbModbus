#include "modbus.h"

#include <ModbusMaster.h>

ModbusMaster g_modBus;

bool ModBus::connect()
{
	// Try USB first
	Serial.begin(115200);
    g_modBus.begin(1, Serial);

    auto res = g_modBus.readInputRegisters(55, 2);	// dummy read total energy

    if(res == g_modBus.ku8MBSuccess)
    	return true;

    delay(1000);

	// Try older serial inverter as a backup if USB failed
	Serial.begin(9600);
	g_modBus.begin(1, Serial);
	res = g_modBus.readInputRegisters(28, 2);		// dummy read total energy

    return res == g_modBus.ku8MBSuccess;
}

int ModBus::readInputRegisters(uint16_t* buffer, uint32_t _address, uint32_t _count)
{
	const auto res = g_modBus.readInputRegisters(_address, _count);
	if (res != g_modBus.ku8MBSuccess)
		return res;
	for(int i=0; i<_count; ++i)
		buffer[i] = g_modBus.getResponseBuffer(i);
	return res;
}

int ModBus::readHoldingRegisters(uint16_t* buffer, uint32_t _address, uint32_t _count)
{
	const auto res = g_modBus.readHoldingRegisters(_address, _count);
	if (res != g_modBus.ku8MBSuccess)
		return res;
	for(int i=0; i<_count; ++i)
		buffer[i] = g_modBus.getResponseBuffer(i);
	return res;
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

		default:										return String("Unknown error code ") + String(errorCode);
	}
}
