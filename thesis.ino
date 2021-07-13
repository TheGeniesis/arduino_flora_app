#include "DEV_Config.h"
#include "TSL2591.h"
UWORD Lux = 0;

void setup() {
DEV_ModuleInit();
Serial.begin(9600);
while(!Serial) delay(1);
Serial.println(123);

TSL2591_Init();
}

void loop() {
 
    //Lux = TSL2591_Read_Lux();
    //Serial.print("Lux = ");
    //Serial.print(Lux);
    //Serial.print("\r\n");
    //TSL2591_SET_LuxInterrupt(50,200);
}
