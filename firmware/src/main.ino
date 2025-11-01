/*
  GreenTech Vacuum – Fake Battery + Rocker Motor Switch
  - OLED shows simulated battery %
  - Drops by 1% every 10 seconds
  - Type 'F' in Serial Monitor to restore 100%
  - Rocker switch (SW2) controls motor ON/OFF input
  - Uses only analogWrite() for PWM (compatible with ESP32 core 3.3.1)
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- pins ----------------
const int PIN_PWM = 25;           // to MDD10A PWM
const int PIN_DIR = 26;           // to MDD10A DIR (set HIGH for forward)
const int PIN_SWITCH_MOTOR = 27;  // your 2nd rocker switch input
const int I2C_SDA = 21;           // OLED SDA
const int I2C_SCL = 22;           // OLED SCL
const uint8_t OLED_ADDR = 0x3C;

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- PWM ----------------
const int TARGET_SPEED_PCT = 65;  // change 35/65/100 etc.
const int PWM_RES_BITS = 8;       // 8-bit (0–255)

// ---------------- fake battery ----------------
int fakePct = 100;
const unsigned long DRAIN_PERIOD_MS = 10000; // 10s per -1%
const int DRAIN_STEP = 1;
unsigned long lastDrainMs = 0;

// ---------------- serial commands ----------------
String cmd;

void printHelp() {
  Serial.println(F("\n=== Commands ==="));
  Serial.println(F("F  -> Set battery to 100%"));
  Serial.println(F("H  -> Help"));
  Serial.println();
}

// ---------------- motor helpers ----------------
void pwmBegin() {
  pinMode(PIN_DIR, OUTPUT);
  digitalWrite(PIN_DIR, HIGH); // forward
  pinMode(PIN_PWM, OUTPUT);

  // Correct ESP32 version:
  analogWriteResolution(PIN_PWM, PWM_RES_BITS);
}

void pwmWritePercent(int pct) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  int duty8 = map(pct, 0, 100, 0, 255);
  analogWrite(PIN_PWM, duty8);
}

// ---------------- UI ----------------
void drawUI(bool motorOn) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("GreenTech Vacuum"));

  display.setTextSize(2);
  display.setCursor(0, 18);
  display.printf("Bat %d%%", fakePct);

  display.setCursor(0, 42);
  if (motorOn)
    display.printf("SPD %d%%", TARGET_SPEED_PCT);
  else
    display.print("MOTOR OFF");

  // battery bar
  int barX = 96, barY = 8, barW = 28, barH = 48;
  display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
  int fillH = map(fakePct, 0, 100, 0, barH - 2);
  display.fillRect(barX + 1, barY + (barH - 1 - fillH), barW - 2, fillH, SSD1306_WHITE);

  display.display();
}

// ---------------- setup/loop ----------------
void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println(F("\n[GreenTech Vacuum] Fake Battery + Switch Motor Control"));
  printHelp();

  pinMode(PIN_SWITCH_MOTOR, INPUT_PULLUP); // rocker switch input
  pwmBegin();
  pwmWritePercent(0); // motor off initially

  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED init failed (addr 0x3C?)"));
  }
  display.clearDisplay(); display.display();

  lastDrainMs = millis();
  drawUI(false);
}

void loop() {
  // 1) fake drain timer
  unsigned long now = millis();
  if (now - lastDrainMs >= DRAIN_PERIOD_MS) {
    fakePct -= DRAIN_STEP;
    if (fakePct < 0)   fakePct = 0;
    if (fakePct > 100) fakePct = 100;
    lastDrainMs = now;
  }

  // 2) rocker switch input (LOW = ON if connected to GND)
  bool motorOn = (digitalRead(PIN_SWITCH_MOTOR) == LOW);
  if (motorOn) {
    pwmWritePercent(TARGET_SPEED_PCT);
  } else {
    pwmWritePercent(0);
  }

  // 3) serial command handler
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (cmd.length()) {
        if (cmd.equalsIgnoreCase("F")) {
          fakePct = 100;
          Serial.println(F("Battery restored to 100%."));
        } else if (cmd.equalsIgnoreCase("H")) {
          printHelp();
        } else {
          Serial.print(F("Unknown: ")); Serial.println(cmd);
          printHelp();
        }
        cmd = "";
      }
    } else {
      cmd += c;
      if (cmd.length() > 16) cmd.remove(0);
    }
  }

  // 4) refresh UI
  static unsigned long lastUi = 0;
  if (now - lastUi >= 100) {
    drawUI(motorOn);
    lastUi = now;
  }
}
