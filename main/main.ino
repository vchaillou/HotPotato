#include <easyMesh.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <stdio.h>

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

bool playerReady = false;

ESP8266WebServer server(WEBSERVER_PORT);
const char * ssid = "Server_Potato";
const char * ContentHtml = "<!DOCTYPE html>\
                        <html>\
                          <head>\
                          <title>Potato Game</title>\
                          </head>\
                          <body>\
                          </body>\
                        </html>"; 
const char * linkJquery = "<link rel='stylesheet' href='//code.jquery.com/ui/1.12.1/themes/base/jquery-ui.css'>\
                           <script src='https://code.jquery.com/jquery-1.12.4.js'></script>\
                           <script src='https://code.jquery.com/ui/1.12.1/jquery-ui.js'></script>";
const char * contentJquery = "<script>\
                                $( function() {\
                                } );\
                          </script>";
const char * contentCss = "<style>\
                              table{\
                                border-style: solid;\ 
                                border-width: 1px;\ 
                                margin-left:40%\
                              }\
                              td{\
                                border-style: solid;\ 
                                border-width: 1px;\ 
                              }\    
                          </style>";
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
  //setupMesh();
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
  Serial.print("Message received:");
  Serial.println(msg);
  String message = msg;
  if(msg.startsWith("NewPlayer ")) {
    message = msg.substring(15);
    Serial.println(message);
    uint32_t playerNode = message.substring(0, message.indexOf(" ")).toInt();
    Serial.println(playerNode);
    message = message.substring(message.indexOf(" ")+1);
    Serial.println(message);
    String playerName = message.substring(7);
    Serial.println(playerName);
    playerList[playerCount].name = playerName;
    playerList[playerCount].node = playerNode;
    playerList[playerCount].num = playerCount;
    playerList[playerCount].nbWon = 0;
    playerList[playerCount].nbLost = 0;
    playerCount++;
    Serial.print("New Player : ");
    Serial.println(playerName);
    mesh.sendSingle(playerNode, String("SetPlayer node:") + String(player.node) + String(" Name:") + String(player.name) + String(" Num:") + String(player.num) + 
      String(" NbWon:") + String(player.nbWon) + String(" NbLost:") + String(player.nbLost) + String(" PlayerCount:") + String(playerCount));
  }
  else if(msg.startsWith("SetPlayer ")) {
    message = msg.substring(15);
    Serial.println(message);
    uint8_t playerNode = message.substring(0, message.indexOf(" ")).toInt();
    Serial.println(playerNode);
    message = message.substring(message.indexOf(" ")+1);
    Serial.println(message);
    String playerName = message.substring(7);
    Serial.println(playerName);
    message = message.substring(message.indexOf(" ")+1);
    //int playerNum, playerNbWon, playerNbLost;
    //char charMessage[message.length()];
    //message.toCharArray(charMessage, message.length());
    //sscanf(charMessage, "Num:%i NbWon:%i NbLost:%i PlayerCount:%i", &playerNum, &playerNbWon, &playerNbLost, &playerCount);
    int playerNum = message.substring(3, message.indexOf(" ")).toInt();
    message = message.substring(message.indexOf(" ")+1);
    int playerNbWon = message.substring(5, message.indexOf(" ")).toInt();
    message = message.substring(message.indexOf(" ")+1);
    int playerNbLost = message.substring(6, message.indexOf(" ")).toInt();
    message = message.substring(message.indexOf(" ")+1);
    playerCount = message.substring(11).toInt();
    playerList[playerNum].num = playerNum;
    playerList[playerNum].name = playerName;
    playerList[playerNum].nbWon = playerNbWon;
    playerList[playerNum].nbLost = playerNbLost;
    playerList[playerNum].node = playerNode;

    player.num = playerCount-1;
    playerList[playerCount-1] = player;
    playerReady = true;
  }
}

void newConnectionCallback(bool adopt) {
  Serial.println("New node found");
}

void webRoot() {
  server.send(200, "text/html", getHTML());
}


String getStatHtml(){
  int numGame = player.nbWon + player.nbLost;
  String str_num = String(player.num);
  String str_lost = String(player.nbLost);
  String str_win = String(player.nbWon);
  String str_all = String(numGame);

  String stat = "<br/>\
                 <table>\
                  <tr>\
                    <td>N°</td>\
                    <td>Name</td>\
                    <td>Number lost</td>\
                    <td>Number win</td>\
                    <td>Number all Game</td>\
                  </tr>\
                  <tr>\
                      <td>"+str_num+"</td>\
                      <td>"+player.name+"</td>\
                      <td>"+str_lost+"</td>\
                      <td>"+str_win+"</td>\
                      <td>"+str_all+"</td>\
                    </tr>\
                </table>";
  return stat;
}

String getHTML() {
  String tmpContentHtml = ContentHtml;
  String tmpContentJquery = contentJquery;
  String tmpLinkJquery = linkJquery;
  String tmpContentCss = contentCss;
  String tmpFormHtml = "<Form style='margin-left:40%' action='/set_name'><label>Name Player</label><input name='name_player'/>\
                    <button type='submit'>Save name</button></Form>";
  String replaceOcc0 = "</title>";
  tmpContentHtml.replace(replaceOcc0, replaceOcc0 + tmpContentJquery); 
  String replaceOcc1 = "<head>";
  tmpContentHtml.replace(replaceOcc1, replaceOcc1 + tmpLinkJquery);
  tmpContentHtml.replace(replaceOcc1, replaceOcc1 + tmpContentCss);
  String replaceOcc2 =  "<body>";
  tmpContentHtml.replace(replaceOcc2, replaceOcc2 + tmpFormHtml);
  String statHtml = getStatHtml();
  
  if(player.name.length() > 0) {
    return "Hello " + player.name + statHtml;
  }
  return tmpContentHtml;
}


void setName() {
  player.name = server.arg(0);
  Serial.println(player.name);
  if(player.nbWon != 0 || player.nbLost != 0){
    server.send(200, "text/html", getHTML() + getStatHtml());
  }
  else{
    server.send(200, "text/html", getHTML());  
  }
  addPlayer();
}

void addPlayer() {
  setupMesh();
  int i=0;

  while(i<30 && !playerReady) {
    mesh.sendBroadcast(String("NewPlayer Node:") + String(player.node) + String(" Player:") + String(player.name));
    delay(1000);
    mesh.update();
    i++;
  }

  while(i<30) {
    delay(1000);
    mesh.update();
    i++;
  }

  Serial.println("Game Start");
  // game start
  
}

