#include <easyMesh.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <stdio.h>

/*********************************/
/***** CONSTANTS AND GLOBALS *****/
/*********************************/

#define MESH_PREFIX     "HOTPOTATO"
#define MESH_PASSWORD   "Zodiaque"
#define MESH_PORT       21147
#define WEBSERVER_PORT  80

#define NETWORK_SSID      "ESGI"
#define NETWORK_PASSWORD  "Reseau-GES"

#define MAX_PLAYERS 10

const int buttonPin = D2; // Button to thow the potato to another player
const int redLightPin = D7;    // Death Indicator
const int buzzerPin = D8;  // Buzzer "explosion"

easyMesh mesh;

// Structure that contains player data
typedef struct {
  String name;
  uint32_t node;    // needed for the sendSingle method
  int num;
  int nbWon;
  int nbLost;
} Player;

Player playerList[MAX_PLAYERS];
int playerCount = 0;

bool hasPotato = false;

bool gameStarted = false;
int timer = -1;             // in seconds

ESP8266WebServer server(WEBSERVER_PORT);

// HTML/CSS/JS Stuff
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
                                  $('#id_button_table').click(function(){\
                                    $('#id_div_table').toggle();\
                                  });\
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

/***************************/
/***** SETUP FUNCTIONS *****/
/***************************/

// Initializes the mesh network
void setupMesh() {
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.setReceiveCallback( &receivedCallback );
  mesh.setNewConnectionCallback( &newConnectionCallback ); 
}

// Intializes the pin modes for the button, the LED and the buzzer
void setupPin() {
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(redLightPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
}

// Connects to the network
// Uses defined constants NETWORK_SSID and NETWORK_PASSWORD for connection
void setupWifi() {
  WiFi.begin(NETWORK_SSID, NETWORK_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }
 Serial.println("Connected !");
 Serial.println(WiFi.localIP());
}

// Initializes the web server with 3 routes :
//  - / (index)
//  - /BeginGameWithPotato
//  - /BeginGameWithoutPotato
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

// Setup that handles all of the above setups
// Also add players
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  setupPin();
  setupMesh();
  setupWifi();
  setupServer();

  // TO CHANGE
  addPlayer(1022050, "Valentin C.");
  addPlayer(13666106, "Christian");
  addPlayer(2652946, "Valentin M.");
  // TODO => VM
}

/*********************/
/***** FUNCTIONS *****/
/*********************/

// Adds a player the player list
// Just needs the node and the name
// Counter is automatically incremented
void addPlayer(uint32_t node, String name) {
  playerList[playerCount].node = node;
  playerList[playerCount].name = name;
  playerList[playerCount].nbWon = 0;
  playerList[playerCount].nbLost = 0;
  playerList[playerCount].num = playerCount;
  playerCount++;
}

/*********************************/
/***** MESH NETWORK HANDLERS *****/
/*********************************/

// Handler called when a message is received from a node of the mesh network
// 3 cases :
//  - the game is not started => it's the timer, so start the game
//  - the game is started and still running (timer > 0) => we get the potato
//  - the game is started but timer is off => we get statistics and stop the game
void receivedCallback(uint32_t from, String &msg) {
  if(!gameStarted) {
    timer = msg.toInt();
    Serial.println(timer);
    gameStarted = true;
    hasPotato = false;
  }
  else if(timer > 0){
    hasPotato = true;
    digitalWrite(redLightPin, HIGH);
  }
  else {
    gameStarted = false;
    Serial.println("Score !");
    for(int i=0 ; i<playerCount; i++) {
      if(playerList[i].node == msg.toInt()) {
        playerList[i].nbLost += 1;
      }
      else {
        playerList[i].nbWon += 1;
      }
    }
    setupWifi();
  }
}

// Handler called when a new node has been found
void newConnectionCallback(bool adopt) {
  Serial.println("New node found");
}

/*****************************************/
/***** HTML AND WEB SERVER FUNCTIONS *****/
/*****************************************/

// Handler for "/" route
void webRoot() {
  server.send(200, "text/html", getHTML());
}

// Returns the HTML component that contains the table with statistics
// Called from the function getHtml()
String getStatHtml(){
  String statHead = "<br/><button id='id_button_table'>Show/Hide</button>\
                 <div id='id_div_table'><table>\
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
  String stat = statHead + statBody + String("</table>" + String("</div>"));
  return stat;
}

// Returns the main HTML component with the two buttons and the statistics
// Buttons implicitely call the beginGameWithPotato and BeginGameWithoutPotato functions
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

// Handler for "/BeginGameWithPotato" route
// Launches the game
// The player begins with the potato
// Disconnects from the wifi network
// Connects to the other players with mesh network
// Initializes the timer with a random number between 10 seconds and 120 seconds
void beginGameWithPotato() {
  server.send(200, "text/plain", "Game will be launched shortly...");
  WiFi.disconnect();
  while(mesh.connectionCount() < playerCount-1) {
    mesh.update();
    delay(1000);
  }
  gameStarted = true;
  hasPotato = true;
  digitalWrite(redLightPin, HIGH);
  timer = random(10, 120);
  String str = String(timer);
  mesh.sendBroadcast(str);
  Serial.println(timer);
}

// Handler for "/BeginGameWithoutPotato" route
// Disconnect from the wifi network so we can connect to mesh network
void beginGameWithoutPotato() {
  server.send(200, "text/plain", "Waiting for the potato...");
  WiFi.disconnect();
}

/*********************/
/***** MAIN LOOP *****/
/*********************/

// If the timer is over, trigger the end of the game and transfer the scores if the player lost
// Else handle the button press if the game is running
// Waits 1 second each time
void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  mesh.update();

  // If the game is finished
  if(timer == 0) {
    if(hasPotato) {
      digitalWrite(buzzerPin, HIGH);
      delay(250);
      digitalWrite(buzzerPin, LOW);

      // Only the loser knows who lost and who won the game
      for(int i=0 ; i<playerCount; i++) {
        if(playerList[i].node == mesh.getChipId()) {
          delay(1000);        // just to be sure the other players finished their game too
          String str = String(playerList[i].node);
          while(mesh.connectionCount() < playerCount-1) {
            mesh.update();
            delay(100);
          }
          Serial.println("Sending Score...");
          mesh.sendBroadcast(str);
          break;
        }
      }
      for(int i=0 ; i<playerCount; i++) {
        if(playerList[i].node == mesh.getChipId()) {
          playerList[i].nbLost += 1;
        }
        else {
          playerList[i].nbWon += 1;
        }
        gameStarted = false;
      }
      setupWifi();
    }
    
    hasPotato = false;
    digitalWrite(redLightPin, LOW);
    timer = -1;
  }
  else if(timer > 0) {
    timer--;
  }
  // End If the game is finished

  int buttonPressed = digitalRead(buttonPin);

  // If button pressed
  if(gameStarted && timer>0 && hasPotato && buttonPressed == LOW) {
    Serial.println("Bouton !");
    for(int i=0 ; i<playerCount ; i++) {
      if(playerList[i].node == mesh.getChipId()) {
        String str = String("YOURETHEPOTATOOWNER");
        while(mesh.connectionCount() < playerCount-1) {
          mesh.update();
          delay(100);
        }
        mesh.sendSingle(playerList[(i+1)%playerCount].node, str);
        digitalWrite(redLightPin, LOW);
        hasPotato = false;
        break;
      }
    }
  }
  // End If button pressed
  delay(1000);
}

