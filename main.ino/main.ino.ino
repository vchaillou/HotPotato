#include <easyMesh.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define MESH_PREFIX     "HOTPOTATO"
#define MESH_PASSWORD   "Zodiaque"
#define MESH_PORT       21147
#define WEBSERVER_PORT  80

#define NETWORK_SSID      "ESGI"
#define NETWORK_PASSWORD  "Reseau-GES"

#define MAX_PLAYERS 10

#define SEND_POTATO_TO_PLAYER_X "To:X Potato"

const int buttonPin = D0; // Button to thow the potato to another player
const int yellowButtonPin = D5; // Potato owner
const int redButtonPin = D7;    // Death Indicator
const int melodyBuzzerPin = D8;  // Buzzer "explosion"

easyMesh mesh;

typedef struct {
  String name;
  uint32_t node;
  int num;
  int nbWon;
  int nbLost;
} Player;

Player potatoOwner;
Player player;
Player playerList[MAX_PLAYERS];
int playerCount = 0;

ESP8266WebServer server(WEBSERVER_PORT);
const char * ssid = "Server_Potato";

void setupMesh() {
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.setReceiveCallback( &receivedCallback );
  mesh.setNewConnectionCallback( &newConnectionCallback );

  player.node = mesh.getChipId();
}

void setupPin() {
    pinMode(buttonPin, INPUT);
    pinMode(yellowButtonPin, OUTPUT);
    pinMode(redButtonPin, OUTPUT);
    pinMode(melodyBuzzerPin, OUTPUT);
}

void setupWifi() {
  WiFi.begin(NETWORK_SSID, NETWORK_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }
 Serial.println("Connected !");
 Serial.println(WiFi.localIP());
}

void setupServer() {
  if(MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", webRoot);
  server.on("/set_name", setName);
  server.begin();
  Serial.println("HTTP server started");
}

void setupMDNS() {
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  setupPin();
  setupMesh();
  setupWifi();
  setupServer();
  setupMDNS();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  mesh.update();
  if(potatoOwner.node == player.node) {
    //Player &chosenPlayer;
    //mesh.sendBroadcast(SEND_POTATO_TO_PLAYER_X.replace("X", ))
  }
}

void receivedCallback(uint32_t from, String &msg) {
  String message = msg;
  if(msg.startsWith("NewPlayer ")) {
    message = msg.substring(16);
    Serial.println(message);
    uint8_t playerNode = message.substring(0, message.indexOf(" ")).toInt();
    Serial.println(playerNode);
    message = message.substring(message.indexOf(" ")+1);
    Serial.println(message);
    String playerName = message.substring(8);
    Serial.println(playerName);
    playerList[playerCount].name = playerName;
    playerList[playerCount].node = playerNode;
    playerList[playerCount].num = playerCount;
    playerList[playerCount].nbWon = 0;
    playerList[playerCount].nbLost = 0;
    playerCount++;
  }
}

void newConnectionCallback(bool adopt) {
  
}

void webRoot() {
  server.send(200, "text/html", getHTML());
}

String getHTML() {
  if(player.name.length() > 0) {
    return "Hello " + player.name;
  }
  return "<Form action='/set_name'><label>Name Player</label><input name='name_player'/>\
          <button type='submit'>Save name</button></Form>";
}

void setName() {
  player.name = server.arg(0);
  Serial.println(player.name);
  server.send(200, "text/html", getHTML());
  addPlayer();
}

void addPlayer() {
  mesh.sendBroadcast(String("NewPlayer Node:") + String(player.node) + String(" Player:") + String(player.name)); 
}

