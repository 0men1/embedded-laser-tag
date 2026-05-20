#include "player_state.h"
#include "ir_comms.h"
#include "display.h"
#include "networking.h"
#include "input.h"

void setup() {
  Serial.begin(115200);

  setupInputs();

  pinMode(STATUS_LED_RED_PIN, OUTPUT);
  pinMode(STATUS_LED_GREEN_PIN, OUTPUT);
  pinMode(STATUS_LED_BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // nitialize led to red (connecting)
  setStatusLED(LED_BRIGHTNESS, 0, 0);

  updateDisplay();

  setupWifi();
  setupServer();

  setupIR();

  // show blue when connected but not registered
  setStatusLED(0, 0, LED_BRIGHTNESS);
  updateDisplay();
}


void loop() {
  handleNetwork();
  handleInputs();
  handleIR();
  handleVisualEffects();
}
