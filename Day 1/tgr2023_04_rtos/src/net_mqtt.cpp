#include "net_mqtt.h"

// constants
#define TAG "net_mqtt"

#define MQTT_BROKER "192.168.1.2"
#define MQTT_PORT 1883
#define MQTT_USER "TGR_GROUP28"
#define MQTT_PASS "NE499V"

// static variables
static WiFiClient wifi_client;
static PubSubClient mqtt_client(wifi_client);
static IPAddress staticIP(192, 168, 1, 91);
static IPAddress gateway(0, 0, 0, 0);
static IPAddress subnet(255, 255, 255, 0);

// connect WiFi and MQTT broker
void net_mqtt_init(char *ssid, char *passwd)
{
    // initialize WiFi
    WiFi.disconnect(true);
    if (!WiFi.config(staticIP, gateway, subnet))
    {
        ESP_LOGI(TAG, "STA Failed to configure");
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passwd);
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGI(TAG, "Connected to %s", ssid);

    // initialize MQTT
    mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
}

// connect and subscribe to topic
void net_mqtt_connect(unsigned int dev_id, char *topic, mqtt_callback_t msg_callback)
{
    String client_id = "TGR_GROUP" + String(dev_id);
    mqtt_client.setCallback(msg_callback);
    mqtt_client.connect(MQTT_USER, client_id.c_str(), MQTT_PASS);
    mqtt_client.subscribe(topic);
    ESP_LOGI(TAG, "Connected to %s", client_id);
}

// publish message to topic
void net_mqtt_publish(char *topic, char *payload)
{
    mqtt_client.publish(topic, payload);
}

// maintain MQTT connection
void net_mqtt_loop(void)
{
    mqtt_client.loop();
}