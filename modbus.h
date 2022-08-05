#pragma once

#include <cstdint>

#include <Arduino.h>

class ModBus
{
public:
	bool connect();

	int readInputRegisters  (uint16_t* _buffer, uint32_t _address, uint32_t _count);
	int readHoldingRegisters(uint16_t* _buffer, uint32_t _address, uint32_t _count);
	String errorToString(int errorCode);
};
