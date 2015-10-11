/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * temperature.c
 * Copyright (C) 2015 Simon Newton
 */

#include "temperature.h"

#include "system_definitions.h"
#include "peripheral/adc/plib_adc.h"
#include "sys/attribs.h"

#include "coarse_timer.h"
#include "syslog.h"

#include "app_settings.h"

static const uint16_t SAMPLING_PERIOD = 10000;  // 1s

// The conversion function is:
//   temp [deci-degrees] = m * sampled_value + c
//
// The MCP9701AT device has:
//  - 400mV @ 0 degrees C
//  - 19.5mV per 1 degree
//
// With 10-bit sampling and a 3v3 ref we have:
//   temp = ((3300 / 1024 * sample - 400) / 19.5 * 10
//
// Which reduces to:
//   temp = 1.6526 * sample - 205.128;
static const float CONVERSION_MULTIPLIER = 1.6526;
static const float CONVERSION_OFFSET = -205.128;

static CoarseTimer_Value g_timer;

struct {
  bool new_sample;  //!< true if there is a new sample
  uint16_t sample_value;  //!< The raw sampled value
  uint16_t temperature;  //!< The temperature in 10ths of a degree.
} g_adc_data;

void __ISR(_ADC_VECTOR, ipl1AUTO) ADCEvent() {
  // AD1CON1bits.ASAM = 0;
  PLIB_ADC_SampleAutoStartDisable(ADC_ID_1);
  SYS_INT_SourceStatusClear(INT_SOURCE_ADC_1);
  SYS_INT_SourceDisable(INT_SOURCE_ADC_1);
  g_adc_data.new_sample = true;

  // g_adc_data.value = ADC1BUF0;
  g_adc_data.sample_value = PLIB_ADC_ResultGetByIndex(ADC_ID_1, 0);
}

void Temperature_Init() {
#ifdef RDM_RESPONDER_TEMPERATURE_SENSOR
  g_timer = CoarseTimer_GetTime();
  g_adc_data.new_sample = false;
  g_adc_data.sample_value = 0;
  g_adc_data.temperature = 0;

  // The pin must be in analog mode - configure this with the harmony
  // configurator plugin.

  // AD1CON1 = 0x00e0;  // auto conversion, 16bit int
  PLIB_ADC_ResultFormatSelect(ADC_ID_1, ADC_RESULT_FORMAT_INTEGER_16BIT);
  PLIB_ADC_ConversionTriggerSourceSelect(
      ADC_ID_1, ADC_CONVERSION_TRIGGER_INTERNAL_COUNT);

  // AD1CHSbits.CH0SA = 2;
  PLIB_ADC_MuxChannel0InputPositiveSelect(
      ADC_ID_1, ADC_MUX_A, ADC_INPUT_POSITIVE_AN2);

  // AD1CSSL = 0;  // no scanning mask

  // The only method Harmony 1.06 provides to set ADCS uses the bus speed, so
  // rather than depend on that, we set it here manually.
  AD1CON3 = 0x0002; // ADCS
  PLIB_ADC_ConversionClockSourceSelect(
      ADC_ID_1, ADC_CLOCK_SOURCE_SYSTEM_CLOCK);
  PLIB_ADC_SampleAcquisitionTimeSet(ADC_ID_1, 0x1f);  // 31 x Tad

  //AD1CON2 = 0;  // VDD & VSS, no scan, mux A
  PLIB_ADC_MuxAInputScanDisable(ADC_ID_1);
  PLIB_ADC_SamplesPerInterruptSelect(ADC_ID_1, ADC_1SAMPLE_PER_INTERRUPT);

  // AD1CON1bits.ADON = 1;
  PLIB_ADC_Enable(ADC_ID_1);

  SYS_INT_VectorPrioritySet(INT_VECTOR_AD1, INT_PRIORITY_LEVEL1);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_AD1, INT_SUBPRIORITY_LEVEL1);
  SYS_INT_SourceStatusClear(INT_SOURCE_ADC_1);
#endif
}

uint16_t Temperature_GetValue(TemperatureSensor sensor) {
  return g_adc_data.temperature;
}

void Temperature_Tasks() {
#ifdef RDM_RESPONDER_TEMPERATURE_SENSOR
  if (CoarseTimer_HasElapsed(g_timer, SAMPLING_PERIOD)) {
    SYS_INT_SourceStatusClear(INT_SOURCE_ADC_1);
    SYS_INT_SourceEnable(INT_SOURCE_ADC_1);

    // AD1CON1bits.ASAM = 1;
    PLIB_ADC_SampleAutoStartEnable(ADC_ID_1);
    g_timer = CoarseTimer_GetTime();
  }

  if (g_adc_data.new_sample) {
    g_adc_data.new_sample = false;
    g_adc_data.temperature = CONVERSION_MULTIPLIER * g_adc_data.sample_value +
        CONVERSION_OFFSET;
    SysLog_Print(SYSLOG_INFO, "%d %d", g_adc_data.sample_value,
                 g_adc_data.temperature);
  }
#endif
}
