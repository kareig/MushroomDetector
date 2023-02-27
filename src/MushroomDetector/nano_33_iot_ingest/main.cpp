#include "Arduino_BHY2Host.h"
#include "stdarg.h"

#define FREQUENCY_HZ 150

#if (FREQUENCY_HZ <= 0)
#error "FREQUENCY_HZ should have a value greater than 0"
#endif
#define INTERVAL_MS (1000 / FREQUENCY_HZ)
#define CONVERT_G_TO_MS2 9.80665f

void ei_printf(const char *format, ...);

Sensor temp(SENSOR_ID_TEMP);
Sensor hum(SENSOR_ID_HUM);
Sensor baro(SENSOR_ID_BARO);
Sensor gas(SENSOR_ID_GAS);

#define DEBUG false

void setup()
{
  Serial.begin(115200);

#if DEBUG
  BHY2Host.debug(Serial);
#endif

  while (!BHY2Host.begin(false, NICLA_VIA_BLE))
  {
  }

  temp.begin(200, 1);
  hum.begin(200, 1);
  baro.begin(200, 1);
  gas.begin(200, 1);
}

void loop()
{
  BHY2Host.update();
  delay(INTERVAL_MS);

  ei_printf("%.2f, %.2f, %.2f, %.2f,", temp.value(), hum.value(), baro.value(), gas.value());
  ei_printf("\r\n");
}

/**
 * @brief      Printf function uses vsnprintf and output using Arduino Serial
 *
 * @param[in]  format     Variable argument list
 */
void ei_printf(const char *format, ...)
{
  static char print_buf[1024] = {0};

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0)
  {
    Serial.write(print_buf);
  }
}
