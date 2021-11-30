// #include "ESP8266WiFi.h"
#include "config.h"
#include "wifi_connection.h"
#include "mqtt.h"
#include "dht_sensor.h"
#include "light_sensor.h"
#include "ArduinoJson.h"
#include "SPI.h"
#include <ezTime.h>
#include <Wire.h>
#include <Adafruit_MCP3008.h>

Adafruit_MCP3008 adc;
Timezone myTZ;

#define MOISURE_CHANNEL 5

// UWORD Lux = 0;
int t = 0;
int ONE_MINUTE = 60;

float getMoisure()
{
  int map_low = 0;
  int map_high = 560;

  return map(adc.readADC(MOISURE_CHANNEL), map_low, map_high, 0, 100);
}

void setup()
{
  Serial.begin(9600);

  connectWithWifi();
  setupMqtt();

  dhtBegin();
  adc.begin();
  lightBegin();

  waitForSync();
	myTZ.setLocation(F("pl"));
}

void loop()
{
  mqttLoop();
	Serial.println();


  DynamicJsonDocument doc(1024);
  doc["message_id"] = "test";
  doc["date"] = myTZ.dateTime("Y-m-d H:i:s");
  doc["humility"] = getMoisure();
  doc["light"] = getLight();
  doc["temperature"] = getTemperature();
  doc["water_level"] = getCurrentWaterAmount();
  doc["device_id"] = DEVICE_ID;
  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  Serial.println(buffer);
  // mqttPublish("amq_topic.measurement", buffer, n)

  delay(1000);
}
