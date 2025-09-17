/*
  battery.h – Battery voltage & percentage utilities
  For GreenTech Solar Vacuum (ESP32)

  Features:
  - Reads pack voltage via resistor divider → ESP32 ADC
  - Converts voltage to % using Li-ion discharge curve
  - Supports 1S or 3S packs (set BATTERY_CELLS accordingly)
*/

#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

// ========= USER CONFIG =========
#define BATTERY_CELLS 3            // Set to 1 for 1S build, 3 for 3S build

// Voltage divider: Vadc = Vbat * (R2 / (R1 + R2))
const float R1 = 100000.0;         // 100k to Vbat
const float R2 = 33000.0;          // 33k to GND
const float DIVIDER_RATIO = (R2 / (R1 + R2)); // ~0.2477 with 100k/33k

// ADC pin & config
const int PIN_BAT_ADC = 34;        // Must be ADC1-capable pin
const int ADC_MAX     = 4095;      // 12-bit resolution
const float ADC_REF_V = 3.30;      // Effective full-scale after attenuation

// Per-cell reference discharge curve (approximate)
struct Vpt { float v; int pct; };
Vpt tableBase[] = {
  {4.20, 100}, {4.00, 85}, {3.90, 75}, {3.80, 60}, {3.70, 50},
  {3.60, 40}, {3.50, 30}, {3.40, 20}, {3.30, 10}, {3.20, 5}, {3.00, 0}
};

// ========= FUNCTIONS =========

// Build scaled table (1S or 3S)
int voltageToPercent(float packV) {
  static Vpt tbl[sizeof(tableBase)/sizeof(tableBase[0])];
  static bool built = false;
  if (!built) {
    for (size_t i = 0; i < sizeof(tableBase)/sizeof(tableBase[0]); ++i) {
      tbl[i].v   = tableBase[i].v * BATTERY_CELLS;
      tbl[i].pct = tableBase[i].pct;
    }
    built = true;
  }

  if (packV >= tbl[0].v) return 100;
  if (packV <= tbl[sizeof(tbl)/sizeof(tbl[0]) - 1].v) return 0;

  for (size_t i = 0; i < sizeof(tbl)/sizeof(tbl[0]) - 1; ++i) {
    if (packV <= tbl[i].v && packV >= tbl[i+1].v) {
      // Linear interpolation
      float x0 = tbl[i].v,   y0 = tbl[i].pct;
      float x1 = tbl[i+1].v, y1 = tbl[i+1].pct;
      float t  = (packV - x1) / (x0 - x1);
      int pct  = (int)(y1 + t * (y0 - y1) + 0.5f);
      if (pct < 0) pct = 0; if (pct > 100) pct = 100;
      return pct;
    }
  }
  return 0;
}

// Read one raw sample
float readPackVoltageRawOnce() {
  analogSetPinAttenuation(PIN_BAT_ADC, ADC_11db);
  int raw = analogRead(PIN_BAT_ADC);
  float vadc = (raw * ADC_REF_V) / ADC_MAX;  // at ADC pin
  return vadc / DIVIDER_RATIO;               // back to pack voltage
}

// Average multiple samples for stability
float readPackVoltageAveraged(uint8_t samples = 10) {
  float sum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    sum += readPackVoltageRawOnce();
    delay(3);
  }
  return sum / samples;
}

#endif // BATTERY_H
