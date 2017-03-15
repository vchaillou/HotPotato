#include <easyMesh.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define LED_POTATO LED_BUILTIN
#define LED_LOSER LED_BUILTIN
//#define BUZZER

#define MESH_PREFIX     "HOTPOTATO"
#define MESH_PASSWORD   "Zodiaque"
#define MESH_PORT       21147
#define WEBSERVER_PORT  80

#define NETWORK_SSID      "ESGI"
#define NETWORK_PASSWORD  "Reseau-GES"

#define MAX_PLAYERS 10

#define SEND_POTATO_TO_PLAYER_X "To:X Potato"

const int buzzButtonPin = D0;
const int yellowButtonPin = D5;  
const int redButtonPin = D7;  
const int melodyBuzzerPin = D8;  

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

ESP8266WebServer server(WEBSERVER_PORT);
const char * ssid = "Server_Potato";

void setupMesh() {
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.setReceiveCallback( &receivedCallback );
  mesh.setNewConnectionCallback( &newConnectionCallback );

  player.node = mesh.getChipId();
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

  pinMode(LED_POTATO, OUTPUT);
  pinMode(LED_LOSER, OUTPUT);

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
  // TODO => ajouter le joueur
}
