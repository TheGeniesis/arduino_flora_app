#include "DEV_Config.h"
#include "TSL2591.h"
#include "ESP8266WiFi.h"
#include "credentials.h"


UWORD Lux = 0;
int t = 0;
int ONE_MINUTE = 60;

void setup() {
  DEV_ModuleInit();
  Serial.begin(9600);

  connectWithWifi();


  TSL2591_Init();
}

void loop() {
    
  if (t % ONE_MINUTE == 0) {  
    int lux = getLightSensorResult();
  }

  if (t == 100) {
    t=0;
  }
  t++;

  delay(1000);
}

inf getLightSensorResult() {
  // lux is an one lumen per square meter
  Lux = TSL2591_Read_Lux();
  Serial.print("Lux = ");
  Serial.print(Lux);
  Serial.print("\r\n");
  TSL2591_SET_LuxInterrupt(50, 200);
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

// https://auth0.com/blog/build-user-signup-counter-with-arduino-part2/
