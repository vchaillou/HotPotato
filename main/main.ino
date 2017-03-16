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
const int buzzerPin = D8;  // Buzzer "explosion"

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
}

void setupPin() {
    pinMode(buttonPin, INPUT);
    pinMode(yellowButtonPin, OUTPUT);
    pinMode(redButtonPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
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
  server.on("/BeginGameWithPotato", beginGameWithPotato);
  server.on("/BeginGameWithoutPotato", beginGameWithoutPotato);
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

  // TO CHANGE
  addPlayer(1022050, "Valentin C.");
  addPlayer(13666106, "Christian");
  // TODO => VM
}

void addPlayer(uint32_t node, String name) {
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
      digitalWrite(buzzerPin, HIGH);
      delay(250);
      digitalWrite(buzzerPin, LOW);
      for(int i=0 ; i<playerCount; i++) {
        if(playerList[i].node == mesh.getChipId()) {
          playerList[i].nbLost += 1;
        }
      }
    }
    else {
      for(int i=0 ; i<playerCount; i++) {
        if(playerList[i].node == mesh.getChipId()) {
          playerList[i].nbWon += 1;
        }
      }
    }
    
    gameStarted = false;
    hasPotato = false;
    digitalWrite(redButtonPin, LOW);
    setupWifi();
    timer = -1;
  }
  else if(timer > 0) {
    timer--;
  }
  
  if(gameStarted && hasPotato && digitalRead(buttonPin) == HIGH) {
    for(int i=0 ; i<playerCount ; i++) {
      if(playerList[i].node == mesh.getChipId()) {
        String str = String("YOURETHEPOTATOOWNER");
        digitalWrite(redButtonPin, LOW);
        mesh.sendSingle(playerList[(i+1)%playerCount].node, str);
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
    digitalWrite(redButtonPin, HIGH);
  }
}

void newConnectionCallback(bool adopt) {
  Serial.println("New node found");
}

void webRoot() {
  server.send(200, "text/html", getHTML());
}


String getStatHtml(){
  String statHead = "<br/>\
                 <table>\
                  <tr>\
                    <td>Ranking</td>\
                    <td>Name</td>\
                    <td>Number lost</td>\
                    <td>Number win</td>\
                    <td>Number all Game</td>\
                  </tr>";
  String statBody;
  for(int i =0; i < playerCount; i++){
    String str_num = String(playerList[i].num);
    String str_lost = String(playerList[i].nbLost);
    String str_win = String(playerList[i].nbWon);
    String str_all = String(playerList[i].nbWon + playerList[i].nbLost);  
    statBody += "<tr>\
                      <td>"+str_num+"</td>\
                      <td>"+playerList[i].name+"</td>\
                      <td>"+str_lost+"</td>\
                      <td>"+str_win+"</td>\
                      <td>"+str_all+"</td>\
                    </tr>"; 
  }
  String stat = statHead + statBody + String("</table>");
  return stat;
}

String getHTML() {
  String tmpContentHtml = ContentHtml;
  String tmpContentJquery = contentJquery;
  String tmpLinkJquery = linkJquery;
  String tmpContentCss = contentCss;
  String tmpFormHtml = "<Form style='margin-left:40%' action='/BeginGameWithPotato'><button type='submit'>Start with potato</button></Form>\
                        <Form style='margin-left:40%' action='/BeginGameWithoutPotato'><button type='submit'>Start without potato</button></Form>";
  String replaceOcc0 = "</title>";
  tmpContentHtml.replace(replaceOcc0, replaceOcc0 + tmpContentJquery); 
  String replaceOcc1 = "<head>";
  tmpContentHtml.replace(replaceOcc1, replaceOcc1 + tmpLinkJquery);
  tmpContentHtml.replace(replaceOcc1, replaceOcc1 + tmpContentCss);
  String replaceOcc2 =  "<body>";
  tmpContentHtml.replace(replaceOcc2, replaceOcc2 + tmpFormHtml);
  String statHtml = getStatHtml();
  
  
  return tmpContentHtml + statHtml;
}

void beginGameWithPotato() {
  server.send(200, "text/plain", "Game is running...");
  WiFi.disconnect();
  delay(10000);
  mesh.update();
  gameStarted = true;
  hasPotato = true;
  digitalWrite(redButtonPin, HIGH);
  timer = random(10, 120);
  String str = String(timer);
  mesh.sendBroadcast(str);
  Serial.println(timer);
}

void beginGameWithoutPotato() {
  server.send(200, "text/plain", "Game is running...");
  WiFi.disconnect();
  setupMesh();
}



