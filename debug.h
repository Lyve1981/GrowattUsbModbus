#pragma once

static constexpr bool g_dryRun = false;

static constexpr bool g_debug = g_dryRun;

class String;

void debugln(const String& msg);
void debug(const String& msg);
