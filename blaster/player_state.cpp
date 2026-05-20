#include <Arduino.h>
#include "player_state.h"
#include "input.h"
#include "display.h"

uint8_t ammo = MAX_AMMO;
uint8_t health = MAX_HEALTH;
uint8_t eliminations = 0;

bool isAlive = true;
bool registered = false;
bool isReloading = false;
bool wifiConnected = false;

void reloadAmmo() {
  if (ammo < MAX_AMMO && isAlive) {
    isReloading = true;
    reloadStartTime = millis();
    tone(BUZZER_PIN, 400, 100);
    updateDisplay();
  }
}
