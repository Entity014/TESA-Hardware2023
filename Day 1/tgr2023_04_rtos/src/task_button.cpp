#include "main.h"
#include "task_button.h"
#include "net_mqtt.h"

// constants
#define TAG "task_button"

#define BTN_PIN 0

// static function prototypes
static void task_button_fcn(void *arg);

// task initialization
void task_button_init()
{
    xTaskCreate(
        task_button_fcn,  /* Task function. */
        "Button Task",    /* String with name of task. */
        2048,             /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        TASK_BUTTON_PRIO, /* Priority of the task. */
        NULL);            /* Task handle. */
    ESP_LOGI(TAG, "task_button created at %d", millis());
}

// task function
void task_button_fcn(void *arg)
{
    uint32_t prev_millis = millis();
    // task initialization
    pinMode(BTN_PIN, INPUT_PULLUP);
    while (1)
    {
        // task function
        evt_msg_t evt_msg = {
            .type = TASK_BUTTON_TYPE,
            .timestamp = 0,
            .pressed = false,
            .value = 0};
        if (!enable_flag)
        {
            if (digitalRead(BTN_PIN) == LOW)
            {
                ESP_LOGI(TAG, "Button pressed at %d", millis());
                if (millis() - prev_millis > 500)
                {
                    prev_millis = millis();
                    evt_msg.pressed = true;
                    xQueueSend(evt_queue, &evt_msg, portMAX_DELAY);
                }
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}