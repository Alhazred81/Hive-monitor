//web_common.h

#pragma once
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

void initWeb();
void loopWeb();