#pragma once

#include <cstdint>

#include <Arduino.h>

class ModBus
{
public:
	bool connect();
	bool isValid() const { return m_valid; }

	int readInputRegisters  (uint16_t* _buffer, uint32_t _address, uint32_t _count);
	int readHoldingRegisters(uint16_t* _buffer, uint32_t _address, uint32_t _count);
	int writeHoldingRegister(uint16_t _address, uint16_t _value);

	String errorToString(int errorCode);

	uint32_t getBaudRate() const { return m_baudRate; }

private:
	bool m_valid = false;
	uint32_t m_baudRate = 0;
};
