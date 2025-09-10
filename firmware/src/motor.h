/*
  motor.h – Cytron MDD10A control (ESP32 LEDC PWM)
  - 20 kHz PWM (above audible)
  - Direction pin HIGH = forward
  - Percent-based power levels
*/

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

// ======= Default pins (can be overridden in initMotor) =======
static int MOTOR_PIN_PWM = 25;
static int MOTOR_PIN_DIR = 26;

// ======= PWM config =======
#define MOTOR_PWM_CH   0
#define MOTOR_PWM_FREQ 20000     // 20 kHz
#define MOTOR_PWM_RES  13        // 13-bit
#define MOTOR_PWM_MAX  ((1 << MOTOR_PWM_RES) - 1)

// Power modes
enum PowerMode { LOW_P = 0, MED_P, HIGH_P, NUM_MODES };

// Default duty percents – tune as needed
static int MOTOR_DUTY_PCT[NUM_MODES] = { 35, 65, 100 };

static bool g_motorEnabled = true;
static PowerMode g_mode = LOW_P;

inline void initMotor(int pwmPin = 25, int dirPin = 26,
                      int freq = MOTOR_PWM_FREQ, int res = MOTOR_PWM_RES, int channel = MOTOR_PWM_CH) {
  MOTOR_PIN_PWM = pwmPin;
  MOTOR_PIN_DIR = dirPin;

  pinMode(MOTOR_PIN_DIR, OUTPUT);
  digitalWrite(MOTOR_PIN_DIR, HIGH); // forward

  ledcSetup(channel, freq, res);
  ledcAttachPin(MOTOR_PIN_PWM, channel);
  ledcWrite(channel, 0);
}

inline void setMotorEnabled(bool en) {
  g_motorEnabled = en;
}

inline bool motorEnabled() {
  return g_motorEnabled;
}

inline void setMotorMode(PowerMode m) {
  g_mode = m;
}

inline PowerMode motorMode() {
  return g_mode;
}

inline const char* motorModeName(PowerMode m) {
  switch (m) {
    case LOW_P:  return "LOW";
    case MED_P:  return "MED";
    case HIGH_P: return "HIGH";
    default:     return "UNK";
  }
}

// Apply current mode → PWM duty
inline void motorApply() {
  int pct  = g_motorEnabled ? MOTOR_DUTY_PCT[g_mode] : 0;
  int duty = (pct * MOTOR_PWM_MAX) / 100;
  digitalWrite(MOTOR_PIN_DIR, HIGH); // forward
  ledcWrite(MOTOR_PWM_CH, duty);
}

// Convenience: set by percentage directly (0–100)
inline void motorSetPercent(int pct) {
  if (!g_motorEnabled) pct = 0;
  if (pct < 0) pct = 0; if (pct > 100) pct = 100;
  int duty = (pct * MOTOR_PWM_MAX) / 100;
  digitalWrite(MOTOR_PIN_DIR, HIGH);
  ledcWrite(MOTOR_PWM_CH, duty);
}

#endif // MOTOR_H
