#include "debug.h"
#include <Arduino.h>

void debugln(const String& msg)
{
	if(g_debug)
		Serial.println(msg);
}

void debug(const String& msg)
{
	if(g_debug)
		Serial.print(msg);
}
