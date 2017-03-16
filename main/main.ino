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

Player playerList[MAX_PLAYERS];
int playerCount = 0;

bool hasPotato = false;

bool gameStarted = false;
int timer = -1;

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
  server.on("/BeginWithPotato", beginWithPotato);
  server.on("/BeginWithoutPotato", beginWithoutPotato);
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

  // TO CHANGE
  addPlayer(1022050, "Valentin C.");
  addPlayer(13666106, "Christian");
  // TODO => VM
}

void addPlayer(uint32_t node, string name) {
  playerList[playerCount].node = node;
  playerList[playerCount].name = name;
  playerList[playerCount].nbWon = 0;
  playerList[playerCount].nbLost = 0;
  playerList[playerCount].num = playerCount;
  playerCount++;
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  mesh.update();

  if(timer == 0) {
    if(hasPotato) {
      // TODO => Turn the buzzer on and off
      for(int i=0 ; i<playerCount : i++) {
        if(playerList[i].node == node) {
          playerList[i].nbLost++;
          break;
        }
      }
    }
    else {
      for(int i=0 ; i<playerCount : i++) {
        if(playerList[i].node == node) {
          playerList[i].nbLost++;
        }
      }
    }
    
    gameStarted = false;
    hasPotato = false;
    // TODO => unlit LED
    setupWifi();
    timer = -1;
  }
  else if(timer > 0) {
    timer--;
  }
  
  if(gameStarted && hasPotato && digitalRead(buttonPin) == HIGH) {
    for(int i=0 ; i<playerCount ; i++) {
      if(playerList[i].node == mesh.getChipId()) {
        // TODO => unlit LED
        mesh.sendSingle(playerList[(i+1)%playerCount].node, "YOURETHEPOTATOOWNER");
        hasPotato = false;
        break;
      }
    }
  }
  delay(1000);
}

void receivedCallback(uint32_t from, String &msg) {
  if(!gameStarted) {
    timer = msg.toInt();
  }
  else {
    hasPotato = true;
    // TODO => lit LED
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

void beginGameWithPotato() {
  setupMesh();
  mesh.update();
  delay(10000);
  gameStarted = true;
  hasPotato = true;
  timer = random(10, 120);
  mesh.sendBroadcast(String(timer));
}

void beginGameWithoutPotato() {
  setupMesh();
}

