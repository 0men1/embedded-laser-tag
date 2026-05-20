#define DECODE_NEC

#include <Arduino.h>
#include <HTTPClient.h>
#include <IRremote.h>
#include "ir_comms.h"
#include "display.h"
#include "networking.h"
#include "player_state.h"
#include "input.h"

void handleIR() {
  if (IrReceiver.decode()) {
	IRData irData = IrReceiver.decodedIRData;
    if (irData.protocol == NEC && isAlive) {
      uint8_t shooterID = irData.address;
      uint8_t damage = irData.command;

	Serial.printf("IR DATA: %d | %d", shooterID, damage);

      if (shooterID != PLAYER_ID) {
        processHit(shooterID, damage);

        // Flash red when hit
        setStatusLED(LED_BRIGHTNESS, 0, 0);
        tone(BUZZER_PIN, 800, 200);

	effectStartTime = millis();
	effectDuration = 200;
	effectActive = true;
      }
    }
    IrReceiver.resume();
  }
}

void sendShot() {
  if (!isAlive)
    return;

  if (ammo < 1) {
    tone(BUZZER_PIN, 200, 300); // o8t of ammo
    return;
  }

  IrSender.sendNEC(PLAYER_ID, 10, 0);
	IrReceiver.start();
  ammo--;

  // Flash white when firing
  setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS);
  tone(BUZZER_PIN, 1000, 100);
	effectStartTime = millis();
	effectDuration = 100;
	effectActive = true;

  updateDisplay();
}

void processHit(uint8_t id, uint8_t dmg) {
  Serial.printf("Hit by player %d with damage %d\n",id, dmg );
  if (dmg >= health) {
    health = 0;
    isAlive = false;
    notifyElimination(PLAYER_ID, id);
  } else {
    health -= dmg;
  }
  updateDisplay();
}

void setupIR() {
  IrSender.begin(IRT_PIN);
  IrReceiver.begin(IRR_PIN);
}
