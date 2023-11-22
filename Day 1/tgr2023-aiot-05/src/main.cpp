#include "main.h"
#include "task_mic.h"
#include "task_mqtt.h"
#include "net_mqtt.h"

// constants
#define TAG "main"

// static function prototypes
void print_memory(void);
static void mqtt_callback(char *topic, byte *payload, unsigned int length);

// static variables
bool enable_flag = true;

StaticJsonDocument<128> cmd_doc;

static char cmd_buf[128];

QueueHandle_t evt_queue;

uint32_t prev_millis1 = millis();

// Setup hardware
void setup()
{
  Serial.begin(115200);
  print_memory();
  evt_queue = xQueueCreate(2, sizeof(evt_msg_t));
  task_mic_init();
  // task_period_init(2000);
  task_mqtt_init(mqtt_callback);
  pinMode(BTN_PIN, INPUT_PULLUP);
}

// Main loop
void loop()
{
  // net_mqtt_loop();
  delay(100);
}

// Print memory information
void print_memory()
{
  ESP_LOGI(TAG, "Total heap: %u", ESP.getHeapSize());
  ESP_LOGI(TAG, "Free heap: %u", ESP.getFreeHeap());
  ESP_LOGI(TAG, "Total PSRAM: %u", ESP.getPsramSize());
  ESP_LOGI(TAG, "Free PSRAM: %d", ESP.getFreePsram());
}

// callback function to handle MQTT message
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  if (digitalRead(BTN_PIN) == LOW)
  {
    if (millis() - prev_millis1 > 500)
    {
      ESP_LOGI(TAG, "Message arrived on topic %s", topic);
      ESP_LOGI(TAG, "Payload: %.*s", length, payload);
      prev_millis1 = millis();
    }
  }
  memcpy(cmd_buf, payload, length);
  cmd_buf[length] = '\0';
  deserializeJson(cmd_doc, cmd_buf);
  if (cmd_doc["ID"] == MQTT_DEV_ID)
  {
    if (cmd_doc["sound"] == true)
    {
      enable_flag = true;
      // ESP_LOGI(TAG, "Got enable command");
    }
    else
    {
      enable_flag = false;
      // ESP_LOGI(TAG, "Got disable command");
    }
  }
}