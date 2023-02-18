/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#define EIDSP_USE_CMSIS_DSP             1  
#define EIDSP_LOAD_CMSIS_DSP_SOURCES    1  

#include <cstdarg>
#define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static inline  
#define __SSAT(ARG1, ARG2) \  
__extension__ \  
({                          \  
  int32_t __RES, __ARG1 = (ARG1); \  
  __ASM volatile ("ssat %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) : "cc" ); \  
  __RES; \  
 })  

/* Includes ---------------------------------------------------------------- */
#include <MushroomDetector_inferencing.h>
#include "Arduino_BHY2Host.h"
#include <U8x8lib.h>
#include <Wire.h>

/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */

void print_inference_result(ei_impulse_result_t result);

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void blink(unsigned long ms);

#define DEBUG false
#define DEBUG_BHY2Host true
#define DEBUG_NN    false
#define ANOMALY_THRESHOLD   0.3                       // Scores above this are an "anomaly"
#define SAMPLING_FREQ_HZ    10                         // Sampling frequency (Hz)
#define SAMPLING_PERIOD_MS  1000 / SAMPLING_FREQ_HZ   // Sampling period (ms)
#define NUM_SAMPLES         EI_CLASSIFIER_RAW_SAMPLE_COUNT  // 50 samples at 10 Hz
#define READINGS_PER_SAMPLE EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME // 4 (4 sensor axes)

Sensor temp(SENSOR_ID_TEMP);
Sensor hum(SENSOR_ID_HUM);
Sensor baro(SENSOR_ID_BARO);
Sensor gas(SENSOR_ID_GAS);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    Serial.begin(115200);

#if DEBUG_BHY2Host
    BHY2Host.debug(Serial);
#endif
  
    while(!BHY2Host.begin(false, NICLA_VIA_BLE)) {}
    
    temp.begin(200,1);
    hum.begin(200,1);
    baro.begin(200,1);
    gas.begin(200,1);

    // initialize OLED display
    u8x8.begin();
    u8x8.setFlipMode(360);
    u8x8.clearDisplay();
    u8x8.display();
}

/**
 * @brief      Arduino main function
 */
void loop()
{
    static float raw_buf[NUM_SAMPLES * READINGS_PER_SAMPLE];
    static signal_t signal;
    int max_idx = 0;
    float max_val = 0.0;
    unsigned long timestamp;
    char str_buf[15];
    char OldLabel[15];
    char NewLabel[15];

    ei_printf("Edge Impulse standalone inferencing (Arduino)\n");

    for (int i = 0; i < NUM_SAMPLES; i++) {
    
        timestamp = millis();
        
        BHY2Host.update();

        // Store raw data into the buffer
        raw_buf[(i * READINGS_PER_SAMPLE) + 0] = temp.value();
        raw_buf[(i * READINGS_PER_SAMPLE) + 1] = hum.value();
        raw_buf[(i * READINGS_PER_SAMPLE) + 2] = baro.value();
        raw_buf[(i * READINGS_PER_SAMPLE) + 3] = gas.value();

        while (millis() < timestamp + SAMPLING_PERIOD_MS);
    }

#if DEBUG
  for (int i = 0; i < NUM_SAMPLES * READINGS_PER_SAMPLE; i++) {
    Serial.print(raw_buf[i]);
    if (i < (NUM_SAMPLES * READINGS_PER_SAMPLE) - 1) {
      Serial.print(", ");
    }
  }
  Serial.println();
#endif


// Turn the raw buffer in a signal which we can the classify
  int err = numpy::signal_from_buffer(raw_buf, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
      ei_printf("ERROR: Failed to create signal from buffer (%d)\r\n", err);
      return;
  }

  // Run inference
  ei_impulse_result_t result = {0};
  err = run_classifier(&signal, &result, DEBUG_NN);
  if (err != EI_IMPULSE_OK) {
      ei_printf("ERROR: Failed to run classifier (%d)\r\n", err);
      return;
  }

  // Print the predictions
  ei_printf("Predictions ");
  ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)\r\n",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    ei_printf("\t%s: %.3f\r\n", 
              result.classification[i].label, 
              result.classification[i].value);
  }

  // Print anomaly detection score
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("\tanomaly score: %.3f\r\n", result.anomaly);
#endif

  // Find maximum prediction
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > max_val) {
      max_val = result.classification[i].value;
      max_idx = i;
    }
  }

  // Print predicted label and value to LCD if not anomalous
  if (result.anomaly < ANOMALY_THRESHOLD) {

    u8x8.setFont(u8x8_font_7x14B_1x2_f);
    sprintf(NewLabel, "%s", result.classification[max_idx].label);

      if (strcmp(NewLabel,OldLabel) != 0) {
          sprintf(OldLabel, "%s", NewLabel);      
          u8x8.drawString(0,0, "         ");
          u8x8.drawString(0,0, String(NewLabel).c_str());
      }

    sprintf(str_buf, "%.3f", max_val);
    u8x8.drawString(0,2, String(str_buf).c_str());
    
    u8x8.setFont(u8x8_font_8x13_1x2_f);
    sprintf(str_buf,"t:%.2f",temp.value());
    u8x8.drawString(0,4, String(str_buf).c_str());
    sprintf(str_buf,"h:%.0f",hum.value());
    u8x8.drawString(0,6, String(str_buf).c_str());
    sprintf(str_buf,"b:%.0f",baro.value());
    u8x8.drawString(8,4, String(str_buf).c_str());
    sprintf(str_buf,"g:%.0f",gas.value());
    u8x8.drawString(8,6, String(str_buf).c_str());
       
  } else {
    u8x8.drawString(4,0, "Unknown");
    ei_printf("\tanomaly score: %.3f\r\n", result.anomaly);
    sprintf(str_buf, "%.3f", result.anomaly);
    u8x8.drawString(4,2, String(str_buf).c_str());
  }

    ei_printf("t: %.2f, h: %.2f, b: %.2f, g: %.2f"
    ,temp.value()
    ,hum.value()
    ,baro.value()
    ,gas.value()
  );
  ei_printf("\r\n");

  ei_printf("EI_CLASSIFIER_RAW_SAMPLE_COUNT: %.d\r\n",EI_CLASSIFIER_RAW_SAMPLE_COUNT);
  ei_printf("EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME: %.d\r\n",EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);

  u8x8.setFont(u8x8_font_open_iconic_embedded_2x2);
  u8x8.drawGlyph(14, 0, 0x4a);

  blink(2500);
}

void blink(unsigned long ms)
{
  unsigned long start = millis();
  unsigned long elapsed = 0;
    while (elapsed < ms) {
      u8x8.setFont(u8x8_font_open_iconic_thing_4x4);
      u8x8.drawGlyph(10, 0, 0x4a);
      u8x8.drawString(10,0," ");
      elapsed = millis() - start;
    }
}