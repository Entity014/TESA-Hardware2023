#ifndef __MAIN_H__
#define __MAIN_H__

// include files
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <Esp.h>
#include <esp_log.h>

// constants
#define TASK_MIC_PRIO 4
#define TASK_PERIOD_PRIO 3
#define TASK_MQTT_PRIO 2

#define TASK_MIC_TYPE 0
#define TASK_PERIOD_TYPE 1

#define WIFI_SSID "Xerophytes"
#define WIFI_PASSWORD "12345678"
#define MQTT_EVT_TOPIC "tgr2023/reai-cmu-manatee/btn_evt"
#define MQTT_CMD_TOPIC "tgr2023/reai-cmu-manatee/cmd"
#define MQTT_DEV_ID 28

#define SAMPLE_RATE 16000
#define SCALE 1e6
#define NUM_SAMPLES 160

#define BTN_PIN 0

// type definitions
typedef struct evt_msg_t
{
    int type;
    uint32_t timestamp;
    bool sound;
    uint32_t duration;
} evt_msg_t;

// shared variables
extern xQueueHandle evt_queue;
extern bool enable_flag;

// public function prototypes

#endif