#include <Arduino.h>

#if defined ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#endif

#include <PubSubClient.h>


#include <izar_wmbus.h>

//uint32_t meterId = 0x12345678;
const char* ssid = "Wifi";
const char* password =  "WifiPassword";
const char* mqttServer = "192.168.1.128";
int mqttPort = 1883;

IzarWmbus reader;

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    // reader.init(meterId);
    reader.init(0);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqttServer, mqttPort);
}

IzarResultData data;

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    FetchResult result = reader.fetchPacket(&data);
    if (result == FETCH_SUCCESSFUL) {
        char topic[64];
        char payload[16];
        sprintf(topic, "water/%d", data.meterId);
        sprintf(payload, "%d", data.waterUsage);

        client.publish(topic, payload);
        
        Serial.print("WatermeterId: ");
        Serial.println(data.meterId, HEX);

        Serial.print("Water consumption: ");
        Serial.println(data.waterUsage);
    } else {
        delay(300);
        reader.ensureRx(); 
    }
}