#pragma once

#define STATUS_LED_RED_PIN 12
#define STATUS_LED_GREEN_PIN 26
#define STATUS_LED_BLUE_PIN 27
#define LED_BRIGHTNESS 255


void setStatusLED(int red, int green, int blue);
void updateDisplay();
