/* 
 * Use this sketch if you want to control nicla from 
 * an external device acting as a host.
 * Here, nicla just reacts to external stimuli coming from
 * the eslov port or through BLE 
 * 
 * NOTE: Remember to choose your Nicla configuration! 
 * If Nicla is used as a Shield, provide the NICLA_AS_SHIELD parameter.
 * If you want to enable just one between I2C and BLE,
 * use NICLA_I2C or NICLA_BLE parameters.
 *
*/

#include "Arduino.h"
#include "Arduino_BHY2.h"
#include "Nicla_System.h"

// Set DEBUG to true in order to enable debug print
#define DEBUG true

long previousMillisBat = 0;
long previousMillisLed = 0;

void setup()
{

#if DEBUG
  Serial.begin(115200);
  //BHY2.debug(Serial);
#endif

  nicla::begin();
  nicla::leds.begin();
  nicla::enableCharge(100);
  BHY2.begin();
  
}

void loop()
{
    unsigned long currentMillis = millis();
    int batteryStatus = nicla::getBatteryStatus();
    
    if(currentMillis - previousMillisBat >= 3000) {
      previousMillisBat = currentMillis;
      Serial.println(String("BatteryStatus: ") + batteryStatus);
    }

    if(currentMillis - previousMillisLed >= 1000) {
      previousMillisLed = currentMillis;
      nicla::leds.setColor(green);
    }

    nicla::leds.setColor(off);

    // Update and then sleep
    BHY2.update(100);
}
