#include <Arduino.h>
#include <HTTPClient.h>
#include <NetworkInterface.h>
#include <WebServer.h>
#include <WiFi.h>
#include "networking.h"
#include "display.h"
#include "player_state.h"

static IPAddress serverIP(192, 168, 4, 1);
static WebServer server(80);
static const char *ssid = "LASERTAG";
static const char *password = "123456789";


void notifyElimination(uint8_t playerID, uint8_t killerID) {
  HTTPClient http;
  http.begin("http://" + serverIP.toString() + "/eliminated");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String data = "id=" + String(PLAYER_ID) + "&eliminatedBy=" + String(lastShooterID);
  http.POST(data);
  http.end();

  tone(BUZZER_PIN, 200, 500);  // eliminated sound
  delay(500);
  tone(BUZZER_PIN, 150, 500);
}

bool registerPlayer() {
  HTTPClient http;
  http.begin("http://" + serverIP.toString() + "/register");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String data = "id=" + String(PLAYER_ID) + "&callbackUrl=http://" + WiFi.localIP().toString();

  int httpResponseCode = http.POST(data);
  if (httpResponseCode > 0) {
    registered = true;
    Serial.println(String(PLAYER_ID) + " registered successfully");
    // Green when registered and ready
    setStatusLED(0, LED_BRIGHTNESS, 0);
  } else {
    Serial.println("Error registering vest");
  }
  http.end();
  return registered;
}

void handleNetwork() {
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  server.handleClient();
  if (WiFi.status() == WL_CONNECTED && !registered) {
    registerPlayer();
    updateDisplay();
  }
}

void setupWifi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}
void setupServer() {
  server.on("/", HTTP_POST, []() {
    server.send(200, "text/plain", "Message received");

    if (server.hasArg("command") && server.arg("command") == "reset") {
      handleResetGame();
    }
  });

  server.begin();
  Serial.println("HTTP server started on " + WiFi.localIP().toString());
}

void handleResetGame() {
  health = MAX_HEALTH;
  ammo = MAX_AMMO;
  isAlive = true;
  isReloading = false;
  eliminations = 0;

  tone(BUZZER_PIN, 1000, 150);
  delay(200);
  tone(BUZZER_PIN, 1500, 150);
  delay(200);
  tone(BUZZER_PIN, 2000, 300);

  setStatusLED(0, LED_BRIGHTNESS, 0);  // Green for ready
  updateDisplay();

  Serial.println("Game reset - Player " + String(PLAYER_ID) + " ready");
}
