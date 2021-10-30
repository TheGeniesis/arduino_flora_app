// #include "Adafruit_TSL2591.h"
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

#define DHTTYPE DHT22     // DHT 22 (AM2302)
#define DHTPIN D6 

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

int MOISUREPIN = D2;
int WATERPIN = D5;

DHT_Unified dht(DHTPIN, DHTTYPE);

const char* mqtt_server = MQTT_SERVER_URL; 
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

WiFiClient client;
PubSubClient mqttClient(client);


// UWORD Lux = 0;
int t = 0;
int ONE_MINUTE = 60;

int getCurrentWaterAmount() {
  return analogRead(WATERPIN);
}


float getTemperature() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));

    return 0;
  }

  Serial.print(F("Temperature: "));
  Serial.print(event.temperature);
  Serial.println(F("°C"));

  return event.temperature;
}

float getLight() {
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
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
  Serial.print(F("[ ")); Serial.print(event.timestamp); Serial.print(F(" ms ] "));
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

  Serial.print(event.light); Serial.println(F(" lux"));
  
  return event.light;
}


int getMoisure()
{
  return analogRead(MOISUREPIN);
}




void watering(int waterAmount) {
  int amountBefore = getCurrentWaterAmount();
  while (10 < getCurrentWaterAmount() && (amountBefore - waterAmount) >= getCurrentWaterAmount()) {
    // Send signal to start watering
      Serial.print("Watering...");

    delay(1000);
  }
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
    delay(5000);
    
  }
}

void setup() {
  Serial.begin(9600);

  connectWithWifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  tsl.begin();
}

void loop() {    
  mqttClient.loop();

  if (!client.connected()) {
    reconnect();
  }


  if (t % ONE_MINUTE == 0) {  
  //  int lux = getLightSensorResult();
  }

  if (t == 100) {
    t=0;
  }
  t++;

  DynamicJsonDocument doc(1024);
  doc["message_id"] = "test";
  doc["date"] = "2021-10-25T16:08:57.264Z";
  doc["humility"] = 0;
  doc["light"] = getLight();
  doc["temperature"] = getTemperature();
  doc["water_level"] = getCurrentWaterAmount();
  doc["device_id"] = DEVICE_ID;
  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  Serial.println("Start: Publishing a data from sensors...");
  mqttClient.publish("amq_topic.measurement", buffer, n);
  Serial.println("Stop: Publishing a data from sensors...");

  delay(1000);
}

