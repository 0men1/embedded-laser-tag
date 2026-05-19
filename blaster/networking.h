#pragma once

#include <cstdint>

void setupWifi();
void setupServer();
void handleNetwork();
void handleResetGame();
void notifyElimination(uint8_t playerID, uint8_t killerID);
