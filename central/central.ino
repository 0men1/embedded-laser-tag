// Team number: #56
// Members: Aditi Ghosh (), Dowlah Ali (), Homen Homa (hhoma2)
// Project Name: Laser Tag
/**
 * Abstract: This project implements an enhanced laser tag system featuring ESP32 microcontrollers for competitive gameplay and detailed statistics tracking. Each player utilizes a single compact unit containing both IR transmitter and receiver components. Beyond traditional laser tag functionality, our implementation offers real-time health/ammo management, multiple game modes, and individual performance metrics. The streamlined architecture enables sophisticated game mechanics while maintaining simplicity, creating a competitive and data-rich laser tag experience with reduced complexity and hardware requirements.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define MAX_PLAYERS 6

#define button1 35
#define button2 32
#define button3 33

const char *ssid = "LASERTAG";
const char *password = "123456789";

WebServer server(80);

struct Player
{
    int id;
    int health;
    int ammo;
    int eliminations;
    String callbackURL;
    bool isActive;
};

Player players[MAX_PLAYERS];
int num_players = 0;

bool gameInProgress = false;

void handleRegisterPlayer();
void handlePlayerElimination();
void notifyPlayer(int playerId, String message);
void resetGame();

void setup()
{
    Serial.begin(115200);

    pinMode(button1, INPUT);
    pinMode(button2, INPUT);
    pinMode(button3, INPUT);

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

void showMenu()
{
    Serial.println("---------Laser Tag Menu---------");
    Serial.println("1. Start the game");
    Serial.println("2. Show players");
    Serial.println("3. Reset the game");
}

void notifyPlayer(int playerId, String data)
{
    HTTPClient http;
    http.begin(players[playerId].callbackURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.POST(data);
    http.end();
}

void handleRegisterPlayer()
{
    if (server.hasArg("id") && server.hasArg("callbackUrl") && num_players < MAX_PLAYERS)
    {
        int id = server.arg("id").toInt();

        players[num_players].id = id;
        players[num_players].isActive = true;
        players[num_players].health = 100;
        players[num_players].ammo = 5;
        players[num_players].eliminations = 0;
        players[num_players].callbackURL = server.arg("callbackUrl");
        num_players++;

        Serial.println("Player registered: " + String(id));
        server.send(200);

        showMenu();
    }
    else
    {
        server.send(400);
    }
}

void handlePlayerElimination()
{
    if (server.hasArg("id") && server.hasArg("eliminatedBy"))
    {
        int id = server.arg("id").toInt();
        int eliminatedBy = server.arg("eliminatedBy").toInt();

        for (Player p : players)
        {
            if (p.id == id)
            {
                players[id].health = 0;
                players[id].ammo = 0;
                players[id].eliminations = 0;
            }

            if (p.id == eliminatedBy)
            {
                p.eliminations++;
            }
        }

        notifyPlayer(eliminatedBy, "Player " + String(id) + " eliminated you!");

        Serial.println("Player " + String(id) + " eliminated by " + String(eliminatedBy));
        server.send(200);
    }
    else
    {
        server.send(400);
    }
}

void loop()
{
    server.handleClient();

    int button1_reading = digitalRead(button1);
    static int last_button1_state = LOW;
    if (button1_reading != last_button1_state && button1_reading == HIGH)
    {
    }
    last_button1_state = button1_reading;

    int button2_reading = digitalRead(button2);
    static int last_button2_state = LOW;
    if (button2_reading != last_button2_state && button2_reading == HIGH)
    {
        Serial.println("-----------PLAYERS-----------");
        for (Player p : players)
        {
            Serial.printf("%d, %d, %s, %s\n", p.id, p.eliminations, p.callbackURL, p.isActive ? "Active" : "Not Active");
        }
    }
    last_button2_state = button2_reading;

    int button3_reading = digitalRead(button3);
    static int last_button3_state = LOW;
    if (button3_reading != last_button3_state && button3_reading == HIGH)
    {
        resetGame();
    }
    last_button3_state = button3_reading;
}