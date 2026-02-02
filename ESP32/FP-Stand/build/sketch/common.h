#line 1 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/common.h"
#pragma once
#include <WiFi.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

//SERVER VARS
const char ssid[] = "ALJ-ESP32-FP";
const char password[] = "123456789";
///

extern AsyncEventSource ev_sink;
extern AsyncWebServer server;

int32_t isAuthenticated(AsyncWebServerRequest *request);


bool initializeCommon();
void attachServerHandlers();
void beginServer();
void setConfigLoader(bool (*loader)(char* key,char* value),bool overwrite = false);
AsyncEventSource& getEventSink();
