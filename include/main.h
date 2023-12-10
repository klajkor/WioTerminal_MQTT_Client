#pragma once

#include "Arduino.h"
//#include <FreeRTOS_SAMD51.h>

// Function defs for main.cpp only
void setup(void);
void loop(void);
void setup_wifi(void);
void mqtt_reconnect(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void battery_status_update_callback(TimerHandle_t xTimer);

