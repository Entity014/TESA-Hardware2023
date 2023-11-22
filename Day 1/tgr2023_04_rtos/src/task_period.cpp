#include "main.h"
#include "task_period.h"
#include "net_mqtt.h"

// constants
#define TAG         "task_period"

#define BTN_PIN     0 

// static function prototypes
static void task_period_fcn(void* arg);

// task initialization
void task_period_init(uint32_t period_ms) {
    static uint32_t arg = period_ms;
    xTaskCreate(
        task_period_fcn,    /* Task function. */
        "Periodic Task",    /* String with name of task. */
        2048,               /* Stack size in bytes. */
        &arg,               /* Parameter passed as input of the task */
        TASK_PERIOD_PRIO,   /* Priority of the task. */
        NULL);              /* Task handle. */
    ESP_LOGI(TAG, "task_period created at %d", millis());
}

// task function
void task_period_fcn(void* arg) {
    // task initialization
    uint32_t period_ms = *((uint32_t*)arg);
    while(1) {
        // task function
        evt_msg_t evt_msg = {
            .type = TASK_PERIOD_TYPE,
            .timestamp = millis(),
            .pressed = false,
            .value = esp_random()
        };
        ESP_LOGI(TAG, "task_period run at %d", millis());
        xQueueSend(evt_queue, &evt_msg, portMAX_DELAY);
        vTaskDelay(period_ms / portTICK_PERIOD_MS);
    }
}
