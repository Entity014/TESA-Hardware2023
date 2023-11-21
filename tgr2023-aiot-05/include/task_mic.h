#ifndef __TASK_BUTTON_H__
#define __TASK_BUTTON_H__

// include files
#include <Arduino.h>
#include <FreeRTOS.h>
#include <esp_log.h>
#include "main.h"
#include "hw_mic.h"

// shared variables

// public function prototypes
void task_mic_init(void);

#endif // __TASK_BUTTON_H__