#include "configfile.h"

#include "LittleFS.h"

#include "debug.h"

String ConfigFile::get(const char* _key, const char* _default)
{
    auto f = LittleFS.open(_key, "r");
    if (!f)
    {
    	debugln("Failed to open file " + String(_key));
        return _default;    	
    }

	String result = "";

    while (f.available())
        result += (char)f.read();

    f.close();

	if(result.length() == 0)
		return _default;

    return result;
}

bool ConfigFile::set(const char* _key, const char* _value)
{
	if(strlen(_key) == 0)
	{
		LittleFS.remove(_key);
		return true;
	}

    File f = LittleFS.open(_key, "w");
    if (!f)
    {
    	debugln("Failed to create file " + String(_key));    	
        return false;
    }

    const auto bytesWritten = f.print(_value);

    f.close();

	if(!bytesWritten)
	{
    	debugln("Failed to write to file " + String(_key));    	
        return false;
	}

    return bytesWritten > 0;
}
