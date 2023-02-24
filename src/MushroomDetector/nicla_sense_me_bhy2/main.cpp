#include "Arduino.h"
#include "Arduino_BHY2.h"
#include "Nicla_System.h"

#define DEBUG false

long previousMillisBat = 0;
int interval = 10000;
void updateLEDBatteryStatus(bool interval, int ms = 1000);
void setLEDBatteryStatus();

void setup()
{
  Serial.begin(115200);

#if DEBUG
  BHY2.debug(Serial);
#endif

  nicla::begin();
  nicla::leds.begin();
  nicla::enableCharge(100);
  updateLEDBatteryStatus(false);
  BHY2.begin();
}

void loop()
{
  BHY2.update();
  updateLEDBatteryStatus(true, interval);
}

void updateLEDBatteryStatus(bool interval, int ms)
{
  unsigned long currentMillis = millis();
  if (!interval)
  {
    setLEDBatteryStatus();
    return;
  }
  else
  {
    if (currentMillis - previousMillisBat >= ms)
    {
      previousMillisBat = currentMillis;
      setLEDBatteryStatus();
    }
  }
}

void setLEDBatteryStatus()
{
  int batteryStatus = nicla::getBatteryStatus();
  if (batteryStatus == 5 || batteryStatus == 4)
  {
    nicla::leds.setColor(green);
  }
  else if (batteryStatus == 3)
  {
    nicla::leds.setColor(yellow);
  }
  else if (batteryStatus == 2)
  {
    nicla::leds.setColor(red);
  }
  else
  {
    nicla::leds.setColor(off);
  }
}
