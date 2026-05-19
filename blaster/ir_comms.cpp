#include <Arduino.h>
#include <HTTPClient.h>
#include <IRremote.h>
#include "ir_comms.h"
#include "display.h"
#include "networking.h"
#include "player_state.h"

void handleIR() {
  if (IrReceiver.decode()) {
    if (isAlive) {
      uint32_t rawData = IrReceiver.decodedIRData.decodedRawData;

      uint8_t shooterID = (rawData >> 8) & 0xFF;
      uint8_t damage = rawData & 0xFF;

      if (shooterID != PLAYER_ID) {
        Serial.println("Hit by player " + String(shooterID) + " with damage " +
                       String(damage));
        lastShooterID = shooterID;
        processHit(shooterID, damage);

        // Flash red when hit
        setStatusLED(LED_BRIGHTNESS, 0, 0);
        tone(BUZZER_PIN, 800, 200);
        delay(200);
        updateDisplay(); // Restore normal LED state
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

  uint32_t rawData = (((uint32_t)PLAYER_ID) << 8) | 10;
  IrSender.sendNECRaw(rawData, 0);
  ammo--;

  // Flash white when firing
  setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS);
  tone(BUZZER_PIN, 1000, 100);
  delay(100);

  updateDisplay();
}

void processHit(uint8_t id, uint8_t dmg) {
  if (dmg >= health) {
    health = 0;
    isAlive = false;
    notifyElimination(PLAYER_ID, lastShooterID);
  } else {
    health -= dmg;
  }
  updateDisplay();
}

void setupIR() {
  IrSender.begin(IRT_PIN);
  IrReceiver.begin(IRR_PIN);
}
