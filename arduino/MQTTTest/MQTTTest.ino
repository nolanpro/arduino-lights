// MQTT test sketch for Arduino Uno R4 WiFi
// Connects to HiveMQ Cloud broker via TLS (port 8883).
// TLS is handled by the ESP32-S3 module's firmware (embedded CA bundle).
// Subscribes to a topic and publishes a test message on boot.

#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "ArduinoMqttClient.h"

// WiFi credentials
const char ssid[] = "foo";
const char pass[] = "foo";

// MQTT broker settings
const char broker[] = "157f4ec42861480695b95d99a47d8ae4.s1.eu.hivemq.cloud";
const int  port     = 8883;

// MQTT credentials
const char mqtt_user[] = "hivemq.webclient.1785990931941";
const char mqtt_pass[] = "xtiLPVKElsAWLzegvl7a@CGRpBr%kYjy";

// Topics
const char subscribeTopic[] = "arduino/in";
const char publishTopic[]   = "arduino/out";

int wifiStatus = WL_IDLE_STATUS;

// Use WiFiSSLClient for TLS (ESP32-S3 handles encryption on-chip)
WiFiSSLClient wifiClient;
MqttClient mqttClient(wifiClient);

unsigned long lastPublish = 0;
bool testMessageSent = false;

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("Firmware: ");
  Serial.println(WiFi.firmwareVersion());
}

void onMqttMessage(int messageSize) {
  Serial.print("Received message on topic: ");
  Serial.println(mqttClient.messageTopic());

  Serial.print("Payload: ");
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port
  }

  Serial.println("\n=== MQTT Test ===");
  Serial.println("Board: Arduino Uno R4 WiFi");

  // Check for WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("ERROR: WiFi module not found!");
    while (true) {
      delay(1000);
    }
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("WARNING: Please upgrade the WiFi firmware");
  }

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  while (wifiStatus != WL_CONNECTED) {
    wifiStatus = WiFi.begin(ssid, pass);
    if (wifiStatus != WL_CONNECTED) {
      delay(2000);
    }
  }

  // Wait for DHCP
  Serial.print("Waiting for IP");
  unsigned long dhcpStart = millis();
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    if (millis() - dhcpStart > 30000) {
      Serial.println("\nERROR: DHCP failed");
      while (true) {
        delay(1000);
      }
    }
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  printWifiStatus();

  // Set MQTT client ID
  mqttClient.setId("ArduinoR4-MQTTTest");

  // Set username/password for MQTT authentication
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);

  // Set message receive callback
  mqttClient.onMessage(onMqttMessage);

  // Connect to MQTT broker
  Serial.print("Connecting to MQTT broker: ");
  Serial.print(broker);
  Serial.print(":");
  Serial.println(port);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("ERROR: MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (true) {
      delay(1000);
    }
  }

  Serial.println("Connected to MQTT broker!");

  // Subscribe to topic
  Serial.print("Subscribing to topic: ");
  Serial.println(subscribeTopic);
  mqttClient.subscribe(subscribeTopic);

  Serial.println("Ready - waiting for messages");
  Serial.println();
}

void loop() {
  // Keep the MQTT connection alive and check for incoming messages
  mqttClient.poll();

  // Send a test message once after boot
  if (!testMessageSent) {
    Serial.print("Publishing test message to ");
    Serial.print(publishTopic);
    Serial.print(": ");

    mqttClient.beginMessage(publishTopic);
    mqttClient.print("Hello from Arduino R4 WiFi! Boot successful.");
    mqttClient.endMessage();

    Serial.println("Hello from Arduino R4 WiFi! Boot successful.");
    testMessageSent = true;
  }

  // Optional: publish periodically
  unsigned long now = millis();
  if (now - lastPublish >= 30000) {
    lastPublish = now;

    Serial.print("Publishing heartbeat to ");
    Serial.print(publishTopic);
    Serial.print(": ");

    mqttClient.beginMessage(publishTopic);
    mqttClient.print("Heartbeat - uptime: ");
    mqttClient.print(now / 1000);
    mqttClient.print("s");
    mqttClient.endMessage();

    Serial.print("Heartbeat - uptime: ");
    Serial.print(now / 1000);
    Serial.println("s");
  }
}