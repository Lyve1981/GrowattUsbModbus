#include "configfile.h"

#include "LittleFS.h"
#include <String.h>

String ConfigFile::get(const char* _key, const char* _default)
{
    auto f = LittleFS.open(_key, "r");
    if (!f)
        return _default;

	String result = "";

    while (f.available())
        result += (char)f.read();

    f.close();
    return result;
}

bool ConfigFile::set(const char* _key, const char* _value)
{
    File f = LittleFS.open(_key, "w");
    if (!f)
        return false;

    const auto bytesWritten = f.print(_value);

    f.close();

    return bytesWritten > 0;
}
