#include "main.h"
#include "task_camera.h"
#include "tgr-hardware2_inferencing.h"
#include "edge-impulse-sdk/dsp/image/image.hpp"

// constants
#define TAG "task_camera"

// static function prototypes
static void task_camera_fcn(void *arg);
static void ei_prepare_feature(uint8_t *img_buf, signal_t *signal);
static int ei_get_feature_callback(size_t offset, size_t length, float *out_ptr);
static void ei_use_result(ei_impulse_result_t result, evt_msg_t *msg);

// static variables
static uint8_t *bmp_buf;

// task initialization
void task_camera_init()
{
    xTaskCreate(
        task_camera_fcn,  /* Task function. */
        "Camera Task",    /* String with name of task. */
        4096,             /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        TASK_CAMERA_PRIO, /* Priority of the task. */
        NULL);            /* Task handle. */
    ESP_LOGI(TAG, "task_camera created at %d", millis());
}

// task function
void task_camera_fcn(void *arg)
{
    hw_camera_init();
    bmp_buf = (uint8_t *)ps_malloc(BMP_BUF_SIZE);
    if (psramInit())
    {
        ESP_LOGI(TAG, "PSRAM initialized");
    }
    else
    {
        ESP_LOGE(TAG, "PSRAM not available");
    }
    while (1)
    {
        static bool press_state = false;
        static uint32_t prev_millis = 0;
        // task function
        evt_msg_t evt_msg = {
            .type = TASK_CAMERA_TYPE,
            .date = 0,
            .timestamp = millis(),
            .pressed = false,
            .value = 0,
            .qub = 0};

        if (capture_enabled || digitalRead(BTN_PIN) == 0)
        {
            if ((millis() - prev_millis > 500) && (press_state == false))
            {
                uint32_t Tstart, elapsed_time;
                uint32_t width, height;
                bool debug_nn = false;
                ei::signal_t signal;
                ei_impulse_result_t result = {0};

                prev_millis = millis();
                Tstart = millis();
                // get raw data
                ESP_LOGI(TAG, "Taking snapshot...");
                // use raw bmp image
                hw_camera_raw_snapshot(bmp_buf, &width, &height);
                elapsed_time = millis() - Tstart;
                ESP_LOGI(TAG, "Snapshot taken (%d) width: %d, height: %d", elapsed_time, width, height);

                // prepare feature
                Tstart = millis();

                // generate feature
                ei_prepare_feature(bmp_buf, &signal);
                elapsed_time = millis() - Tstart;
                ESP_LOGI(TAG, "Feature taken (%d)", elapsed_time);

                // run classifier
                Tstart = millis();
                run_classifier(&signal, &result, debug_nn);
                elapsed_time = millis() - Tstart;
                ESP_LOGI(TAG, "Classification done (%d)", elapsed_time);

                // use result
                ei_use_result(result, &evt_msg);
                xQueueSend(evt_queue, &evt_msg, portMAX_DELAY);
                capture_enabled = false;
                press_state = true;
            }
        }
        else
        {
            if (press_state)
            {
                press_state = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// prepare feature
void ei_prepare_feature(uint8_t *img_buf, signal_t *signal)
{
    signal->total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal->get_data = &ei_get_feature_callback;
    if ((EI_CAMERA_RAW_FRAME_BUFFER_ROWS != EI_CLASSIFIER_INPUT_WIDTH) || (EI_CAMERA_RAW_FRAME_BUFFER_COLS != EI_CLASSIFIER_INPUT_HEIGHT))
    {
        ei::image::processing::crop_and_interpolate_rgb888(
            img_buf,
            EI_CAMERA_RAW_FRAME_BUFFER_COLS,
            EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
            img_buf,
            EI_CLASSIFIER_INPUT_WIDTH,
            EI_CLASSIFIER_INPUT_HEIGHT);
    }
}

// get feature callback
int ei_get_feature_callback(size_t offset, size_t length, float *out_ptr)
{
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0)
    {
        out_ptr[out_ptr_ix] = (bmp_buf[pixel_ix] << 16) + (bmp_buf[pixel_ix + 1] << 8) + bmp_buf[pixel_ix + 2];

        // go to the next pixel
        out_ptr_ix++;
        pixel_ix += 3;
        pixels_left--;
    }
    return 0;
}

// use result from classifier
void ei_use_result(ei_impulse_result_t result, evt_msg_t *msg)
{
    ESP_LOGI(TAG, "Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
             result.timing.dsp, result.timing.classification, result.timing.anomaly);
    bool bb_found = result.bounding_boxes[0].value > 0;
    float value = 0;
    for (size_t ix = 0; ix < result.bounding_boxes_count; ix++)
    {
        auto bb = result.bounding_boxes[ix];
        String temp = bb.label;
        if (temp.charAt(temp.length() - 1) == 'Y')
        {
            temp[temp.length() - 1] = '\0';
            int randomIndex = random(3);
            float result;
            switch (randomIndex)
            {
            case 0:
                result = 0.2f;
                break;
            case 1:
                result = 0.5f;
                break;
            case 2:
                result = 0.8f;
                break;
            }
            value = temp.toInt() + result;
        }
        if (bb.value == 0)
        {
            continue;
        }
        ESP_LOGI(TAG, "%s (%f) [ x: %u, y: %u, width: %u, height: %u ]", bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
    }
    ESP_LOGI(TAG, "%f", value);
    msg->value = value;
    msg->timestamp = millis();
    if (!bb_found)
    {
        ESP_LOGI(TAG, "No valid");
    }
}
