#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define DHTPIN D5


    DHT_Unified dht(DHTPIN, DHTTYPE);

 
void dhtBegin()
{
    dht.begin();
}

float getTemperature()
{
    // Get temperature event and print its value.
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    delay(4000);

    if (isnan(event.temperature))
    {
        Serial.println(F("Error reading temperature!"));

        return 0;
    }

    return event.temperature;
}
