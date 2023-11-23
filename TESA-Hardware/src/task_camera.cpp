#include "main.h"
#include "task_camera.h"
#include "tgr2023-aiot-03_inferencing.h"
#include "edge-impulse-sdk/dsp/image/image.hpp"

// constants
#define TAG "task_camera"

// static function prototypes
static void task_camera_fcn(void *arg);
static void ei_prepare_feature(uint8_t *img_buf, signal_t *signal);
static int ei_get_feature_callback(size_t offset, size_t length, float *out_ptr);
static void ei_use_result(ei_impulse_result_t result, evt_msg_t *msg);
static void findMax(ei_impulse_result_bounding_box_t arr[], int size, uint32_t &maxValue, int &maxIndex);
static char getLastCharacter(const char *str);

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
            .timestamp = 0,
            .value = 0,
        };

        if (capture_enabled)
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
    ei_impulse_result_bounding_box_t arrDataInt[result.bounding_boxes_count];
    ei_impulse_result_bounding_box_t arrDataFloat[result.bounding_boxes_count];
    ei_impulse_result_bounding_box_t arrDataBar[result.bounding_boxes_count];
    uint32_t arrValueInt[result.bounding_boxes_count];
    float32_t arrValueFloat[result.bounding_boxes_count];
    uint32_t maxY_F, maxY_B;
    int maxIn_F, maxIn_B;
    for (size_t ix = 0; ix < result.bounding_boxes_count; ix++)
    {
        auto bb = result.bounding_boxes[ix];
        char lastChar = getLastCharacter(bb.label);
        String temp = bb.label;
        if (lastChar == 'Y')
        {
            arrDataInt[ix] = bb;
            temp[strlen(temp.c_str()) - 1] = '\0';
            arrValueInt[ix] = temp.toInt();
        }
        else if (lastChar == 'B')
        {
            arrDataFloat[ix] = bb;
            temp[strlen(temp.c_str()) - 1] = '\0';
            arrValueFloat[ix] = temp.toFloat() / 1e2;
        }
        else
        {
            arrDataBar[ix] = bb;
        }
        if (bb.value == 0)
        {
            continue;
        }
        ESP_LOGI(TAG, "%s (%f) [ x: %u, y: %u, width: %u, height: %u ]", bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
    }
    findMax(arrDataFloat, sizeof(arrDataFloat) / sizeof(arrDataFloat[0]), maxY_F, maxIn_F);
    findMax(arrDataBar, sizeof(arrDataBar) / sizeof(arrDataBar[0]), maxY_B, maxIn_B);
    ESP_LOGI(TAG, "%u [%u : %u]", maxY_F, maxIn_F, arrValueInt[maxIn_F]);
    ESP_LOGI(TAG, "%u [ %u ]", maxY_B, maxIn_B);
    if (maxY_F > maxY_B)
    {
        msg->value = arrValueInt[maxIn_F] + arrValueFloat[maxIn_B] - 0.1;
    }
    else
    {
        msg->value = arrValueInt[maxIn_F] + arrValueFloat[maxIn_B];
    }
    msg->timestamp = millis();
    if (!bb_found)
    {
        ESP_LOGI(TAG, "No objects found");
    }
}

void findMax(ei_impulse_result_bounding_box_t arr[], int size, uint32_t &maxValue, int &maxIndex)
{
    // Initialize the minimum value with the first element
    maxValue = arr[0].y;
    maxIndex = 0;

    // Iterate through the array to find the maximum value and its index
    for (int i = 1; i < size; i++)
    {
        if (arr[i].y > maxValue)
        {
            maxValue = arr[i].y;
            maxIndex = i;
        }
    }
}

char getLastCharacter(const char *str)
{
    // Get the length of the string
    int length = strlen(str);

    // Check if the string is not empty
    if (length > 0)
    {
        // Return the last character
        return str[length - 1];
    }
    else
    {
        // Return a default value or handle the empty string case
        return '\0'; // null character
    }
}