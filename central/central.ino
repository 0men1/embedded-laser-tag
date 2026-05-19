#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define MAX_PLAYERS 6

const char *ssid = "LASERTAG";
const char *password = "123456789";

WebServer server(80);

struct Player {
  int id;
  int health;
  int ammo;
  int eliminations;
  String callbackURL;
  bool isActive;
};


const uint32_t debounce_delay = 50;

struct Button {
  const uint8_t pin;
  uint32_t lastDebounceTime;
  int state;
  int lastState;
};

Button buttons[3] = {
  { 22, 0, LOW, LOW },
  { 23, 0, LOW, LOW },
  { 33, 0, LOW, LOW }
};

Player players[MAX_PLAYERS];
int num_players = 0;

bool gameInProgress = false;

void handleRegisterPlayer();
void handlePlayerElimination();
void notifyPlayer(int playerId, String message);
void resetGame();

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) {
    pinMode(buttons[i].pin, INPUT_PULLDOWN);
  }

  WiFi.softAP(ssid, password);
  Serial.print("IP: " + WiFi.softAPIP().toString());

  server.on("/register", HTTP_POST, handleRegisterPlayer);
  server.on("/eliminated", HTTP_POST, handlePlayerElimination);

  server.begin();
  Serial.println("Server started");

  showMenu();
}

void resetGame() {
  gameInProgress = false;

  for (int i = 0; i < num_players; ++i) {
    HTTPClient http;
    http.begin(players[i].callbackURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.POST("command=reset");
    http.end();
  }

  Serial.println("Game reset - notified players");
}

void showMenu() {
  Serial.println("---------Laser Tag Menu---------");
  Serial.println("1. Start the game");
  Serial.println("2. Show players");
  Serial.println("3. Reset the game");
}

void notifyPlayer(int playerId, String data) {
  HTTPClient http;
  http.begin(players[playerId].callbackURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(data);
  http.end();
}

void handleRegisterPlayer() {
  if (server.hasArg("id") && server.hasArg("callbackUrl") && num_players < MAX_PLAYERS) {
    int id = server.arg("id").toInt();

    players[num_players].id = id;
    players[num_players].isActive = true;
    players[num_players].health = 100;
    players[num_players].ammo = 5;
    players[num_players].eliminations = 0;
    players[num_players].callbackURL = server.arg("callbackUrl");
    num_players++;

    Serial.printf("Player registered: %d", id);
    server.send(200);

    showMenu();
  } else {
    server.send(400);
  }
}

void handlePlayerElimination() {
  if (server.hasArg("id") && server.hasArg("eliminatedBy")) {
    int id = server.arg("id").toInt();
    int eliminatedBy = server.arg("eliminatedBy").toInt();

    int eliminatedIdx = -1, killerIdx = -1;
    for (int i = 0; i < num_players; i++) {
      if (players[i].id == id) eliminatedIdx = i;
      if (players[i].id == eliminatedBy) killerIdx = i;
    }

    if (eliminatedIdx != -1) {
      players[eliminatedIdx].health = 0;
      players[eliminatedIdx].ammo = 0;
      players[eliminatedIdx].isActive = false;
    }
    if (killerIdx != -1) {
      players[killerIdx].eliminations++;
    }

    notifyPlayer(killerIdx, "Player " + String(id) + " eliminated you!");
    Serial.printf("Player %d eliminated by %d\n", id, eliminatedBy);
    server.send(200);
  } else {
    server.send(400);
  }
}

void executeButtonAction(int buttonIndex) {
  if (buttonIndex == 0) {
    // Action for Button 1
  } else if (buttonIndex == 1) {
    Serial.println("-----------PLAYERS-----------");

    for (int i = 0; i < num_players; i++) {
      Serial.printf("%d, %d, %s, %s\n",
                    players[i].id,
                    players[i].eliminations,
                    players[i].callbackURL.c_str(),
                    players[i].isActive ? "Active" : "Not Active");
    }
  } else if (buttonIndex == 2) {
    resetGame();
  }
}

void loop() {
  server.handleClient();

  for (int i = 0; i < 3; i++) {
    int reading = digitalRead(buttons[i].pin);

    if (reading != buttons[i].lastState) {
      buttons[i].lastDebounceTime = millis();
    }

    if ((millis() - buttons[i].lastDebounceTime) > debounce_delay) {
      if (reading != buttons[i].state) {
        buttons[i].state = reading;

        if (buttons[i].state == HIGH) {
          executeButtonAction(i);
        }
      }
    }
    buttons[i].lastState = reading;
  }
}
