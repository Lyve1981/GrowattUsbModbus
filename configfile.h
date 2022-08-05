#pragma once

class String;

class ConfigFile
{
public:
	String get(const char* _key, const char* _default);
	bool set(const char* _key, const char* _value);
};
