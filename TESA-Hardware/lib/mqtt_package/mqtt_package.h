#ifndef MQTT_PACKAGE_H
#define MQTT_PACKAGE_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

class MqttPackages
{
public:
    MqttPackages(const char *ssid, const char *password, const char *mqtt_server, const char *mqtt_client_id, const char *mqtt_topic_prefix);
    void setup(PubSubClient client);
    void loop();
    template <class autoType>
    void sent_data(String path, String arrStr[], autoType arrValue[], int arrSize);

private:
    static void callback(char *topic, byte *payload, unsigned int length);
    void reconnect();
    const char *ssid_;
    const char *password_;
    const char *mqtt_server_;
    const char *mqtt_client_id_;
    const char *mqtt_topic_prefix_;
    PubSubClient client_;
};

#endif