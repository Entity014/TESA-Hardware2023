#include <mqtt_package.h>

MqttPackages::MqttPackages(const char *ssid, const char *password, const char *mqtt_server, const char *mqtt_client_id, const char *mqtt_topic_prefix) : ssid_(ssid), password_(password), mqtt_server_(mqtt_server), mqtt_client_id_(mqtt_client_id), mqtt_topic_prefix_(mqtt_topic_prefix)
{
}

void MqttPackages::callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void MqttPackages::setup(PubSubClient client)
{
    client_ = client;
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid_);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_, password_);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client_.setServer(mqtt_server_, 1883);
    client_.setCallback(callback);
    client_.setBufferSize(2048);
}

void MqttPackages::reconnect()
{
    while (!client_.connected())
    {

        Serial.print("Attempting MQTT connection...");
        if (client_.connect(mqtt_client_id_))
        {
            Serial.println("connected");

            char subscribe_topic[100];
            sprintf(subscribe_topic, "%s/status/", mqtt_topic_prefix_);
            client_.subscribe(subscribe_topic);
            Serial.println("subscribed");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client_.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

template <class autoType>
void MqttPackages::sent_data(String path, String arrStr[], autoType arrValue[], int arrSize)
{
    char topic[128];
    char payload[100];
    char out[128];
    StaticJsonDocument<256> doc;
    for (int i = 0; i < arrSize; i++)
    {
        doc[arrStr[i]] = arrValue[i];
    }
    serializeJson(doc, out);
    sprintf(topic, "%s/%s/", mqtt_topic_prefix_, path);
    sprintf(payload, out);
    client_.publish(topic, payload);
}

void MqttPackages::loop()
{
    if (!client_.connected())
    {
        reconnect();
    }
    client_.loop();
}