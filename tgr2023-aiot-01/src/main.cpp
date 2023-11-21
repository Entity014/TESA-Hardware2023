#include <main.h>
#include <hw_mic.h>
#include <SimpleKalmanFilter.h>

#define TAG "main"
#define SAMPLE_RATE 16000
#define SCALE 1e6
#define NUM_SAMPLES 160

static unsigned int num_samples = NUM_SAMPLES;
static int32_t samples[NUM_SAMPLES]; // ? Array equal Pointer

SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);

bool sound = true;

void setup()
{
  Serial.begin(115200);
  hw_mic_init(SAMPLE_RATE);
}

void loop()
{
  int32_t sample_avg = 0;
  float sound = 0;
  num_samples = NUM_SAMPLES;

  ESP_LOGI(TAG, "loop at %d", millis());
  hw_mic_read(samples, &num_samples);

  for (int i = 0; i < num_samples; i++)
  {
    sample_avg += samples[i] / SCALE;
  }
  sound = (float)sample_avg / num_samples;
  // Serial.println(simpleKalmanFilter.updateEstimate(abs(sound)));
  Serial.println(sound);
  delay(1);
}
