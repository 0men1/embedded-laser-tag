#include <Arduino.h>
#include "display.h"
#include "player_state.h"

void setStatusLED(int red, int green, int blue) {
  analogWrite(STATUS_LED_RED_PIN, red);
  analogWrite(STATUS_LED_GREEN_PIN, green);
  analogWrite(STATUS_LED_BLUE_PIN, blue);
}

void updateDisplay() {
  if (wifiConnected) {
    setStatusLED(LED_BRIGHTNESS, 0, 0); // Red - Disconnected
  } else if (!registered) {
    setStatusLED(0, 0, LED_BRIGHTNESS); // Blue - connecting to server
  } else if (isReloading) {
    setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS, 0); // Yellow - reloading
  } else if (!isAlive) {
    setStatusLED(LED_BRIGHTNESS, 0, LED_BRIGHTNESS); // Purple - eliminated
  } else {
    if (health <= 25) {
      // Low health - orange
      setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS / 2, 0);
    } else if (ammo <= 1) {
      // Low ammo - yellow
      setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS, 0);
    } else {
      // Normal state - green
      setStatusLED(0, LED_BRIGHTNESS, 0);
    }
  }
}
