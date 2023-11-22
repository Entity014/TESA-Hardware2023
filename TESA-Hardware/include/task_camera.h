#ifndef __TASK_CAMERA_H__
#define __TASK_CAMERA_H__

// include files
#include <Arduino.h>
#include <FreeRTOS.h>
#include <esp_log.h>
#include "main.h"
#include "hw_camera.h"
// shared variables

// public function prototypes
void task_camera_init(void);

#endif