#include <Arduino.h>
#include "input.h"
#include "display.h"
#include "player_state.h"
#include "ir_comms.h"

long reloadStartTime = 0;

void setupInputs() {
	pinMode(TRIGGER_PIN, INPUT);
	pinMode(RELOAD_PIN, INPUT);
}

uint32_t effectStartTime = 0;
uint32_t effectDuration = 0;
bool effectActive;

void handleVisualEffects() {
	if (effectActive && (millis() - effectStartTime >= effectDuration)) {
		effectActive = false;
		updateDisplay();
	}
}

void handleInputs() {
  int trigger_reading = digitalRead(TRIGGER_PIN);
  static int last_trigger_state = LOW;
  if (trigger_reading != last_trigger_state && trigger_reading == HIGH) {
    sendShot();
  }
  last_trigger_state = trigger_reading;

  int reload_reading = digitalRead(RELOAD_PIN);
  static int last_reload_state = LOW;
  if (reload_reading != last_reload_state && reload_reading == HIGH) {
    reloadAmmo();
  }
  last_reload_state = reload_reading;

  if (isReloading && millis() - reloadStartTime >= 1000) {
	ammo = MAX_AMMO;
	isReloading = false;
	updateDisplay();
  }
}
