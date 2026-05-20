#pragma once

#include <cstdint>

#define PLAYER_ID 1
#define MAX_AMMO 5
#define MAX_HEALTH 100

#define BUZZER_PIN 32

#define IRT_PIN 21
#define IRR_PIN 15

#define TRIGGER_PIN 23
#define RELOAD_PIN 22

extern bool isAlive;
extern bool registered;

extern uint8_t ammo;
extern uint8_t health;
extern bool isReloading;
extern uint8_t eliminations;

extern bool wifiConnected;

void reloadAmmo();
