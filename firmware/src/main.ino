/*
  GreenTech Solar Vacuum – main.ino (Modular)
  - Uses: battery.h, display.h, motor.h, button.h
  - OLED shows pack voltage & percentage
  - Single push button: short-press cycles LOW→MED→HIGH, long-press toggles motor enable
  - Cytron MDD10A motor driver (PWM + DIR), 20 kHz PWM
  - Battery cells count & divider live in battery.h (set BATTERY_CELLS there)

  Wiring (suggested):
  - ESP32 GPIO25  -> MDD10A PWM
  - ESP32 GPIO26  -> MDD10A DIR (HIGH = forward)
  - ESP32 GPIO27  -> Push Button to GND (uses INPUT_PULLUP)
  - ESP32 SDA 21  -> OLED SDA (I2C)
  - ESP32 SCL 22  -> OLED SCL (I2C)
  - ESP32 GPIO34  -> Battery sense via divider (R1 to Vbat, R2 to GND)
  - LM2596 5 V    -> ESP32 5V (or clean 3.3 V to 3V3)

  Notes:
  - Set BATTERY_CELLS (1 or 3) in battery.h
  - Tune MOTOR_DUTY_PCT[] in motor.h for LOW/MED/HIGH
*/

#include <Arduino.h>

#include "battery.h"  // readPackVoltageAveraged(), voltageToPercent()
#include "display.h"  // initOLED(), drawOLED()
#include "motor.h"    // initMotor(), motorApply(), motorModeName(), etc.
#include "button.h"   // initButton(), buttonTask()

// ---------- Optional: customize pins here (must match your wiring) ----------
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;
static const int MOTOR_PWM_PIN = 25;
static const int MOTOR_DIR_PIN = 26;
static const int BUTTON_PIN    = 27;

void setup() {
  // Serial (optional for debugging)
  Serial.begin(115200);
  delay(50);
  Serial.println("\n[GreenTech Vacuum] Booting...");

  // OLED
  if (!initOLED(I2C_SDA_PIN, I2C_SCL_PIN)) {
    Serial.println("OLED init failed (address 0x3C?). Continuing...");
  } else {
    Serial.println("OLED ready.");
  }

  // Motor (20 kHz PWM), pins can be changed above
  initMotor(MOTOR_PWM_PIN, MOTOR_DIR_PIN);
  Serial.println("Motor driver ready.");

  // Button (short press = cycle, long press = toggle ON/OFF)
  initButton(BUTTON_PIN);
  Serial.println("Button ready.");

  // (Optional) tune LOW/MED/HIGH duty in motor.h -> MOTOR_DUTY_PCT[]
}

void loop() {
  // Handle push button (debounce + short/long press)
  buttonTask();

  // Apply motor output according to current mode & enable state
  motorApply();

  // Battery read & UI
  float vbat = readPackVoltageAveraged(12);  // take 12 samples for stability
  int   pct  = voltageToPercent(vbat);

  char modeStr[16];
  snprintf(modeStr, sizeof(modeStr), "%s%s",
           motorModeName(motorMode()),
           motorEnabled() ? "" : " OFF");

  drawOLED(vbat, pct, modeStr);

  delay(100);  // UI refresh pacing
}
