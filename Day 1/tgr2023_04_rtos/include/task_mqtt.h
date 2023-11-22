#ifndef __TASK_MQTT_H__
#define __TASK_MQTT_H__

// include files
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include "net_mqtt.h"
#include <esp_log.h>

// shared variables

// public function prototypes
void task_mqtt_init(mqtt_callback_t mqtt_callback);

#endif  // __TASK_MQTT_H__