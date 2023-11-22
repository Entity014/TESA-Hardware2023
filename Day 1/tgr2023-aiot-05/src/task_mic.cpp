#include "main.h"
#include "task_mic.h"
#include "net_mqtt.h"
#include <SimpleKalmanFilter.h>
// constants
#define TAG "task_mic"

// static function prototypes
static void task_mic_fcn(void *arg);

// task initialization
void task_mic_init()
{
    xTaskCreate(
        task_mic_fcn,  /* Task function. */
        "Mic Task",    /* String with name of task. */
        2048,          /* Stack size in bytes. */
        NULL,          /* Parameter passed as input of the task */
        TASK_MIC_PRIO, /* Priority of the task. */
        NULL);         /* Task handle. */
    ESP_LOGI(TAG, "task_button created at %d", millis());
}

// task function
void task_mic_fcn(void *arg)
{
    SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);
    static unsigned int num_samples = NUM_SAMPLES;
    static int32_t samples[NUM_SAMPLES]; // ? Array equal Pointer
    uint32_t prev_millis = millis();
    hw_mic_init(SAMPLE_RATE);
    float pre_sound = 0;
    bool once = true;
    bool onceT = true;
    while (1)
    {
        // task function
        evt_msg_t evt_msg = {
            .type = TASK_MIC_TYPE,
            .timestamp = 0,
            .sound = false,
            .duration = 0};

        float sound = 0;
        int32_t sample_avg = 0;
        uint32_t duration_time = 0;
        num_samples = NUM_SAMPLES;
        hw_mic_read(samples, &num_samples);

        for (int i = 0; i < num_samples; i++)
        {
            sample_avg += samples[i] / SCALE;
        }
        sound = (float)sample_avg / num_samples;
        // ESP_LOGI(TAG, "test %d", sound);
        sound = simpleKalmanFilter.updateEstimate(abs(sound));
        // Serial.println(sound);

        if (sound > 2)
        {
            if (once)
            {
                prev_millis = millis();
                ESP_LOGI(TAG, "Start Ah");
                evt_msg.timestamp = millis();
                evt_msg.sound = true;
                evt_msg.duration = duration_time;
                once = false;
                onceT = true;
            }
        }
        else
        {
            if (onceT)
            {
                duration_time = millis() - prev_millis;
                once = true;
                onceT = false;
            }
            if (duration_time != 0)
            {
                ESP_LOGI(TAG, "Stop Ah %d", duration_time);
                evt_msg.timestamp = millis();
                evt_msg.sound = false;
                evt_msg.duration = duration_time;
                pre_sound = sound;
            }
        }
        xQueueSend(evt_queue, &evt_msg, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}