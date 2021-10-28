// #include "Adafruit_TSL2591.h"
#include "ESP8266WiFi.h"
#include "config.h"
#include "ArduinoJson.h"
#include "PubSubClient.h"
 
const char* mqtt_server = MQTT_SERVER_URL; 
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

WiFiClient client;
PubSubClient mqttClient(client);


// UWORD Lux = 0;
int t = 0;
int ONE_MINUTE = 60;



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
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  connectWithWifi();
  mqttClient.setServer(mqtt_server, 1883);

  // TSL2591_Init();
}

void loop() {    
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
  doc["light"] = 0;
  doc["temperature"] = 0;
  doc["water_level"] = 0;
  doc["device_id"] = "4b08f72d-ea82-45c7-b4ae-8e9452201ab1";
  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  Serial.println("Start: Publishing a data from sensors...");
  mqttClient.publish("amq_topic.measurement", buffer, n);
  Serial.println("Stop: Publishing a data from sensors...");





  delay(1000);
}




// int getLightSensorResult() {
//   // lux is an one lumen per square meter
//   Lux = TSL2591_Read_Lux();
//   Serial.print("Lux = ");
//   Serial.println(Lux);
//   TSL2591_SET_LuxInterrupt(50, 200);

//   return Lux;
// }
