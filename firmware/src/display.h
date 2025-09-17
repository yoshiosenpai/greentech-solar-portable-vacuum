/*
  display.h – OLED Display Utilities
  For GreenTech Solar Vacuum (ESP32 + SSD1306)

  Features:
  - Initializes OLED
  - Draws system info: voltage, %, power mode
  - Battery bar indicator on right side
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========== CONFIG ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C   // Common SSD1306 I2C address (check your module)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ========== FUNCTIONS ==========

// Call once in setup()
bool initOLED(int sdaPin = 21, int sclPin = 22) {
  Wire.begin(sdaPin, sclPin);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    return false;  // failed to init
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("OLED Ready"));
  display.display();
  delay(1000);
  return true;
}

// Draws full UI
void drawOLED(float vbat, int pct, const char* modeStr) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Title
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("GreenTech Vacuum"));

  // Voltage
  display.setTextSize(2);
  display.setCursor(0, 14);
  display.printf("V: %.2f", vbat);

  // Battery %
  display.setCursor(0, 34);
  display.printf("Bat %d%%", pct);

  // Mode
  display.setCursor(0, 52);
  display.print(modeStr);

  // Battery bar (right side)
  int barX = 96, barY = 8, barW = 28, barH = 48;
  display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
  int fillH = map(pct, 0, 100, 0, barH - 2);
  display.fillRect(barX + 1, barY + (barH - 1 - fillH), barW - 2, fillH, SSD1306_WHITE);

  display.display();
}

#endif // DISPLAY_H
