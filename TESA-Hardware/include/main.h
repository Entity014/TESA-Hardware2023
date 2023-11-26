#ifndef __MAIN_H__
#define __MAIN_H__

// include files
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <Esp.h>
#include <esp_log.h>

// constants
#define TASK_CAMERA_PRIO 3
#define TASK_MQTT_PRIO 2

#define TASK_CAMERA_TYPE 0

#define WIFI_SSID "TGR17_2.4G"
#define WIFI_PASSWORD ""
#define MQTT_EVT_TOPIC "tgr2023/reai-cmu-manatee/water_evt"
#define MQTT_CMD_TOPIC "tgr2023/reai-cmu-manatee/cmd"
#define MQTT_DEV_ID 28

#define SAMPLE_RATE 16000
#define SCALE 1e6
#define NUM_SAMPLES 160

#define BTN_PIN 0

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 240
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3
#define BMP_BUF_SIZE (EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE)

// type definitions
typedef struct evt_msg_t
{
    int type;
    int date;
    uint32_t timestamp;
    bool pressed;
    float value;
    float qub;
} evt_msg_t;

// shared variables
extern xQueueHandle evt_queue;
extern bool capture_enabled;

// public function prototypes

#endif