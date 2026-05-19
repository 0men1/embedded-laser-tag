#include "player_state.h"
#include "ir_comms.h"
#include "display.h"
#include "networking.h"

unsigned long reloadStartTime = 0;

void reloadAmmo();
void handleResetGame();

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(RELOAD_PIN, INPUT);

  pinMode(STATUS_LED_RED_PIN, OUTPUT);
  pinMode(STATUS_LED_GREEN_PIN, OUTPUT);
  pinMode(STATUS_LED_BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // nitialize led to red (connecting)
  setStatusLED(LED_BRIGHTNESS, 0, 0);

  setupIR();

  updateDisplay();

  setupWifi();
  setupServer();

  // show blue when connected but not registered
  setStatusLED(0, 0, LED_BRIGHTNESS);
  updateDisplay();
}

void loop() {
  handleNetwork();

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

  handleIR();
}

void reloadAmmo() {
  if (ammo < MAX_AMMO && isAlive) {
    isReloading = true;
    reloadStartTime = millis();
    tone(BUZZER_PIN, 400, 100);
    updateDisplay();
  }
}
