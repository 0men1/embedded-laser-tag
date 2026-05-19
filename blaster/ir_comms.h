#pragma once

#include <cstdint>

void setupIR();
void sendShot();
void handleIR();
void processHit(uint8_t id, uint8_t dmg);
void processElimination();
