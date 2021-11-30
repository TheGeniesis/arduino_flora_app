#include "Adafruit_TSL2591.h"
#include <Adafruit_Sensor.h>

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
void lightBegin()
{
    tsl.begin();
}

float getLight()
{
    // You can change the gain on the fly, to adapt to brighter/dimmer light situations
    // tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
    tsl.setGain(TSL2591_GAIN_MED); // 25x gain
    // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
    // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

    // Changing the integration time gives you a longer time over which to sense light
    // longer timelines are slower, but are good in very low light situtations!
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
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
        (event.light < -4294966000.0))
    {
        /* If event.light = 0 lux the sensor is probably saturated */
        /* and no reliable data could be generated! */
        /* if event.light is +/- 4294967040 there was a float over/underflow */
        Serial.println(F("Invalid data (adjust gain or timing)"));
        return 0;
    }

    return event.light;
}
