#ifndef __TASK_PERIOD_H__
#define __TASK_PERIOD_H__

// include files
#include <Arduino.h>
#include <FreeRTOS.h>
#include <esp_random.h>
#include <esp_log.h>

// shared variables

// public function prototypes
void task_period_init(uint32_t period_ms);

#endif  // __TASK_PERIOD_H__