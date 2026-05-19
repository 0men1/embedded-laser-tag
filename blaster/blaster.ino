#include <IRremote.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

// PINS
#define IRT_PIN 21
#define IRR_PIN 15

#define TRIGGER_PIN 23
#define RELOAD_PIN 22

#define STATUS_LED_RED 12
#define STATUS_LED_GREEN 26
#define STATUS_LED_BLUE 27

#define BUZZER_PIN 32

// PLAYER STATS
#define PLAYER_ID 1
#define MAX_AMMO 5
#define MAX_HEALTH 100

#define LED_BRIGHTNESS 255

const char *ssid = "LASERTAG";
const char *password = "123456789";
IPAddress serverIP(192, 168, 4, 1);
WebServer server(80);

uint8_t eliminations = 0;
uint8_t ammo = MAX_AMMO;
uint8_t health = MAX_HEALTH;
uint8_t lastShooterID;
bool isAlive = true;
bool registered = false;
bool isReloading = false;

void sendShot();
void reloadAmmo();
void updateDisplay();
bool registerPlayer();
void processElimination();
void processHit(uint8_t id, uint8_t dmg);
void setStatusLED(int red, int green, int blue);
void handleResetGame();
void setupServer();

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

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(RELOAD_PIN, INPUT);

  pinMode(STATUS_LED_RED, OUTPUT);
  pinMode(STATUS_LED_GREEN, OUTPUT);
  pinMode(STATUS_LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // nitialize led to red (connecting)
  setStatusLED(LED_BRIGHTNESS, 0, 0);

  IrSender.begin(IRT_PIN);
  IrReceiver.begin(IRR_PIN);


  updateDisplay();

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  setupServer();

  // show blue when connected but not registered
  setStatusLED(0, 0, LED_BRIGHTNESS);
  updateDisplay();
}

void loop() {
  server.handleClient();

  if (WiFi.status() == WL_CONNECTED && !registered) {
    registerPlayer();
    updateDisplay();
  }

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

  if (IrReceiver.decode()) {
    if (isAlive) {
      uint32_t rawData = IrReceiver.decodedIRData.decodedRawData;

      uint8_t shooterID = (rawData >> 8) & 0xFF;
      uint8_t damage = rawData & 0xFF;

      if (shooterID != PLAYER_ID) {
        Serial.println("Hit by player " + String(shooterID) + " with damage " + String(damage));
        lastShooterID = shooterID;
        processHit(shooterID, damage);

        // Flash red when hit
        setStatusLED(LED_BRIGHTNESS, 0, 0);
        tone(BUZZER_PIN, 800, 200);
        delay(200);
        updateDisplay();  // Restore normal LED state
      }
    }
    IrReceiver.resume();
  }
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

void setStatusLED(int red, int green, int blue) {
  analogWrite(STATUS_LED_RED, red);
  analogWrite(STATUS_LED_GREEN, green);
  analogWrite(STATUS_LED_BLUE, blue);
}

void updateDisplay() {
  if (WiFi.status() != WL_CONNECTED) {
    setStatusLED(LED_BRIGHTNESS, 0, 0);  // Red - Disconnected
  } else if (!registered) {
    setStatusLED(0, 0, LED_BRIGHTNESS);  // Blue - connecting to server
  } else if (isReloading) {
    setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS, 0);  // Yellow - reloading
  } else if (!isAlive) {
    setStatusLED(LED_BRIGHTNESS, 0, LED_BRIGHTNESS);  // Purple - eliminated
  } else {
    if (health <= 25) {
      // Low health - orange
      setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS / 2, 0);
    } else if (ammo <= 1) {
      // Low ammo - yellow
      setStatusLED(LED_BRIGHTNESS, LED_BRIGHTNESS, 0);
    } else {
      // Normal state - green
      setStatusLED(0, LED_BRIGHTNESS, 0);
    }
  }
}

void reloadAmmo() {
  if (ammo < MAX_AMMO && isAlive) {
    isReloading = true;
    updateDisplay();

    tone(BUZZER_PIN, 400, 100);
    delay(1000);  // reloading

    ammo = MAX_AMMO;
    isReloading = false;
    updateDisplay();
  }
}

void sendShot() {
  if (!isAlive)
    return;

  if (ammo < 1) {
    tone(BUZZER_PIN, 200, 300);  // o8t of ammo
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
    processElimination();
  } else {
    health -= dmg;
  }
  updateDisplay();
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

void processElimination() {
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
