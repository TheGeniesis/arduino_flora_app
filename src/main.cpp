#include "ESP8266WiFi.h"
#include "config.h"
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "SPI.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include "Adafruit_TSL2591.h"
#include <Adafruit_MCP3008.h>

#define DHTTYPE DHT22     // DHT 22 (AM2302)
#define DHTPIN D5
#define PUMP_DIGITAL_PIN D8
#define MOISURE_CHANNEL 5
#define WATER_PIN A0

const int POMP_SPEED_SEC = 18; // 18ml/sec
const int WATER_BOX_SIZE  = 300;
const int WATER_SENSOR_MAX = 460;

const char* mqtt_server = MQTT_SERVER_URL; 
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
Adafruit_MCP3008 adc;
DHT_Unified dht(DHTPIN, DHTTYPE);
WiFiClient client;
PubSubClient mqttClient(client);

// UWORD Lux = 0;
int t = 0;
int ONE_MINUTE = 60;

float getCurrentWaterAmount() {
    delay(500);
    float signalValue = analogRead(WATER_PIN);
    delay(500);
    signalValue = signalValue + analogRead(WATER_PIN);
    delay(500);
    signalValue = (signalValue + analogRead(WATER_PIN))/3;
    
    int percentValue = 0;
    
    if (signalValue <= 250) {
      percentValue = 0;
    } else if (signalValue <= 370) {
      percentValue = 10;
    } else if (signalValue <= 385) {
      percentValue = 10;
    } else if (signalValue <= 400) {
      percentValue = 20;
    } else if (signalValue <= 420) {
      percentValue = 30;
    } else if (signalValue <= 430) {
      percentValue = 40;
    } else if (signalValue <= 452) {
      percentValue = 50;
    } else if (signalValue <= 457) {
      percentValue = 60;
    } else if (signalValue <= 462) {
      percentValue = 70;
    } else if (signalValue <= 465) {
      percentValue = 80;
    } else if (signalValue <= 470) {
      percentValue = 90;
    } else {
      percentValue = 100;
    } 

    return percentValue * WATER_BOX_SIZE / 100;
}

float getMoisure()
{
  int map_low = 0;
  int map_high = 380;

  return map(adc.readADC(MOISURE_CHANNEL), map_low, map_high, 0, 100);
}

float getTemperature() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  delay(4000);

  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));

    return 0;
  }

  return event.temperature;
}

float getLight() {
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  /* Display the results (light is measured in lux) */
  if ((event.light == 0) |
      (event.light > 4294966000.0) | 
      (event.light <-4294966000.0))
  {
    /* If event.light = 0 lux the sensor is probably saturated */
    /* and no reliable data could be generated! */
    /* if event.light is +/- 4294967040 there was a float over/underflow */
    Serial.println(F("Invalid data (adjust gain or timing)"));
    return 0;

  }
  
  return event.light;
}

void watering(int waterAmount) {
  float amountBefore = getCurrentWaterAmount();
  digitalWrite(PUMP_DIGITAL_PIN, HIGH);
  int tries = 0;
  float loopLimit = waterAmount/POMP_SPEED_SEC + 2;
  while (10 < getCurrentWaterAmount() && (amountBefore - waterAmount) >= getCurrentWaterAmount() && tries <= loopLimit) {
    tries++;
  }
  digitalWrite(PUMP_DIGITAL_PIN, LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);

  watering(doc["waterAmount"]);
}

void connectWithWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  Serial.println("In reconnect...");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("dev.dawe ", mqtt_user, mqtt_pass)) {
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

void setup() {
  Serial.begin(9600);

  digitalWrite(PUMP_DIGITAL_PIN, LOW);
  pinMode(PUMP_DIGITAL_PIN, OUTPUT);

  connectWithWifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  dht.begin();
  adc.begin();
  tsl.begin();
}

void loop() {  
  mqttClient.loop();

  if (!client.connected()) {
    reconnect();
  }

  DynamicJsonDocument doc(1024);
  doc["message_id"] = "test";
  doc["date"] = "date";
  doc["humility"] = getMoisure();
  doc["light"] = getLight();
  doc["temperature"] = getTemperature();
  doc["water_level"] = getCurrentWaterAmount();
  doc["device_id"] = DEVICE_ID;
  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  Serial.println(buffer);
  Serial.println("Start: Publishing a data from sensors...");
  // mqttClient.publish("amq_topic.measurement", buffer, n);
  Serial.println("Stop: Publishing a data from sensors...");

  delay(1000);
}

