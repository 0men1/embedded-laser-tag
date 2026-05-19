#include "player_state.h"

uint8_t ammo = MAX_AMMO;
uint8_t health = MAX_HEALTH;
uint8_t lastShooterID = 0;
uint8_t eliminations = 0;

bool isAlive = true;
bool registered = false;
bool isReloading = false;
bool wifiConnected = false;
