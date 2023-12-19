#pragma once

#include "Arduino.h"

#define ENABLE_DEBUG_PRINT (1)

#define DELAY_MAIN_LOOP ((TickType_t)((10) / portTICK_PERIOD_MS))

#define PERIOD_BATTERY_STATUS_UPDATE ((TickType_t)((15000) / portTICK_PERIOD_MS))

#define DELAY_MAIN_LOOP_MILLISEC ((uint32_t)5000)
