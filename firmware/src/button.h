/*
  button.h – Single button handler (short press / long press)
  - Connect button between pin and GND; uses INPUT_PULLUP
  - Short press  => cycle mode (LOW -> MED -> HIGH)
  - Long press   => toggle motor enable
*/

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "motor.h"   // uses PowerMode & setters

// Defaults (override in initButton)
static int BTN_PIN = 27;

// Timing (ms)
static unsigned long BTN_DEBOUNCE_MS = 120;
static unsigned long BTN_LONG_MS     = 800;

// Internal state
static bool           btnLast = HIGH;
static unsigned long  btnPressStart = 0;

inline void initButton(int pin = 27, unsigned long debounceMs = 120, unsigned long longMs = 800) {
  BTN_PIN = pin;
  BTN_DEBOUNCE_MS = debounceMs;
  BTN_LONG_MS = longMs;
  pinMode(BTN_PIN, INPUT_PULLUP);
  btnLast = digitalRead(BTN_PIN);
}

// Call frequently (e.g., each loop)
inline void buttonTask() {
  bool s = digitalRead(BTN_PIN);
  unsigned long now = millis();

  // pressed
  if (btnLast == HIGH && s == LOW) {
    btnPressStart = now;
  }

  // released
  if (btnLast == LOW && s == HIGH) {
    unsigned long held = now - btnPressStart;
    if (held > BTN_DEBOUNCE_MS) {
      if (held >= BTN_LONG_MS) {
        // Long press: toggle motor enable
        setMotorEnabled(!motorEnabled());
      } else {
        // Short press: next mode
        PowerMode m = motorMode();
        m = (PowerMode)((m + 1) % NUM_MODES);
        setMotorMode(m);
      }
    }
  }
  btnLast = s;
}

#endif // BUTTON_H
