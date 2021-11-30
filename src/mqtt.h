#include "config.h"
#include "PubSubClient.h"
#include "ArduinoJson.h"

const char *mqtt_server = MQTT_SERVER_URL;
const char *mqtt_user = MQTT_USER;
const char *mqtt_pass = MQTT_PASSWORD;


#define PUMP_DIGITAL_PIN D8
#define WATER_PIN A0

const int POMP_SPEED_SEC = 18; // 18ml/sec
const int WATER_BOX_SIZE = 300;
const int WATER_SENSOR_MAX = 460;

PubSubClient mqttClient(client);


float getCurrentWaterAmount()
{
  delay(500);
  float signalValue = analogRead(WATER_PIN);
  delay(500);
  signalValue = signalValue + analogRead(WATER_PIN);
  delay(500);
  signalValue = (signalValue + analogRead(WATER_PIN)) / 3;

  int percentValue = 0;

  if (signalValue <= 250)
  {
    percentValue = 0;
  }
  else if (signalValue <= 370)
  {
    percentValue = 10;
  }
  else if (signalValue <= 385)
  {
    percentValue = 10;
  }
  else if (signalValue <= 400)
  {
    percentValue = 20;
  }
  else if (signalValue <= 420)
  {
    percentValue = 30;
  }
  else if (signalValue <= 430)
  {
    percentValue = 40;
  }
  else if (signalValue <= 452)
  {
    percentValue = 50;
  }
  else if (signalValue <= 457)
  {
    percentValue = 60;
  }
  else if (signalValue <= 462)
  {
    percentValue = 70;
  }
  else if (signalValue <= 465)
  {
    percentValue = 80;
  }
  else if (signalValue <= 470)
  {
    percentValue = 90;
  }
  else
  {
    percentValue = 100;
  }

  return percentValue * WATER_BOX_SIZE / 100;
}


void watering(int waterAmount)
{
  float amountBefore = getCurrentWaterAmount();
  digitalWrite(PUMP_DIGITAL_PIN, HIGH);
  int tries = 0;
  float loopLimit = waterAmount / POMP_SPEED_SEC + 2;
  while (10 < getCurrentWaterAmount() && (amountBefore - waterAmount) >= getCurrentWaterAmount() && tries <= loopLimit)
  {
    tries++;
  }
  digitalWrite(PUMP_DIGITAL_PIN, LOW);
}


void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);

  watering(doc["waterAmount"]);
}



void setupMqtt()
{
  digitalWrite(PUMP_DIGITAL_PIN, LOW);
  pinMode(PUMP_DIGITAL_PIN, OUTPUT);

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
}

void reconnect()
{
  // Loop until we're reconnected
  Serial.println("In reconnect...");
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("dev.dawe ", mqtt_user, mqtt_pass))
    {
      Serial.println("connected");
      mqttClient.subscribe("amq_topic.watering");
      return;
    }

    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 5 seconds");
    delay(1000);
  }
}

void mqttLoop()
{
  mqttClient.loop();

  if (!client.connected())
  {
    reconnect();
  }
}

void mqttPublish(const char *topic, const uint8_t *payload, unsigned int plength, boolean retained)
{
  Serial.println("Start: Publishing a data from sensors...");
  // mqttClient.publish("amq_topic.measurement", buffer, n);
  Serial.println("Stop: Publishing a data from sensors...");
}
