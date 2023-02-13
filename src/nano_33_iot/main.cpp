#include "Arduino_BHY2Host.h"

Sensor temperatureSensor(SENSOR_ID_TEMP);
Sensor humiditySensor(SENSOR_ID_HUM);
Sensor pressureSensor(SENSOR_ID_BARO);
Sensor gasSensor(SENSOR_ID_GAS);

#define DEBUG true

long previousMillis = 0;
long interval = 3000; 

void setup() {
  Serial.begin(115200);  

#if DEBUG
  BHY2Host.debug(Serial);
#endif
  
  while(!BHY2Host.begin(false, NICLA_VIA_BLE)) {}
  temperatureSensor.begin(200,1);
  humiditySensor.begin(200,1);
  pressureSensor.begin(200,1);
  gasSensor.begin(200,1);
}

void loop() {  
  unsigned long currentMillis = millis();
  BHY2Host.update();   
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.println(String("temperature: ") + String(temperatureSensor.value(),3));
    Serial.println(String("humidity: ") + String(humiditySensor.value(),3));
    Serial.println(String("pressure: ") + String(pressureSensor.value(),3));
    Serial.println(String("gas: ") + String(gasSensor.value(),3));
    Serial.println();
  }
}





