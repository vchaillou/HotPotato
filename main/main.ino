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
  server.on("/BeginWithPotato", beginGameWithPotato);
  server.on("/BeginWithoutPotato", beginGameWithoutPotato);
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
      // TODO => Turn the buzzer on and off
      for(int i=0 ; i<playerCount; i++) {
        if(playerList[i].node == mesh.getChipId()) {
          playerList[i].nbLost++;
          break;
        }
      }
    }
    else {
      for(int i=0 ; i<playerCount; i++) {
        if(playerList[i].node == mesh.getChipId()) {
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
        String str = String("YOURETHEPOTATOOWNER");
        // TODO => unlit LED
        mesh.sendSingle(playerList[(i+1)%playerCount].node, str);
        hasPotato = false;
        break;
      }
    }
  }
  delay(1000);

  // Button code
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
   }
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
  String tmpFormHtml = "<Form style='margin-left:40%' action='/BeginGameWithPotato'><button type='submit' name='game' value='start'>Start with potato</button></Form>\
                        <Form style='margin-left:40%' action='/BeginGameWithoutPotato'><button type='submit' name='game' value='start'>Start without potato</button></Form>";
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
  setupMesh();
  mesh.update();
  delay(10000);
  gameStarted = true;
  hasPotato = true;
  timer = random(10, 120);
  String str = String(timer);
  mesh.sendBroadcast(str);
}

void beginGameWithoutPotato() {
  setupMesh();
}



void gameOver() {
  digitalWrite(redLedPin, HIGH);
  playGameOverTheme();
}


#define  C0 16.35
#define Db0 17.32
#define D0  18.35
#define Eb0 19.45
#define E0  20.60
#define F0  21.83
#define Gb0 23.12
#define G0  24.50
#define Ab0 25.96
#define LA0 27.50
#define Bb0 29.14
#define B0  30.87
#define C1  32.70
#define Db1 34.65
#define D1  36.71
#define Eb1 38.89
#define E1  41.20
#define F1  43.65
#define Gb1 46.25
#define G1  49.00
#define Ab1 51.91
#define LA1 55.00
#define Bb1 58.27
#define B1  61.74
#define C2  65.41
#define Db2 69.30
#define D2  73.42
#define Eb2 77.78
#define E2  82.41
#define F2  87.31
#define Gb2 92.50
#define G2  98.00
#define Ab2 103.83
#define LA2 110.00
#define Bb2 116.54
#define B2  123.47
#define C3  130.81
#define Db3 138.59
#define D3  146.83
#define Eb3 155.56
#define E3  164.81
#define F3  174.61
#define Gb3 185.00
#define G3  196.00
#define Ab3 207.65
#define LA3 220.00
#define Bb3 233.08
#define B3  246.94
#define C4  261.63
#define Db4 277.18
#define D4  293.66
#define Eb4 311.13
#define E4  329.63
#define F4  349.23
#define Gb4 369.99
#define G4  392.00
#define Ab4 415.30
#define LA4 440.00
#define Bb4 466.16
#define B4  493.88
#define C5  523.25
#define Db5 554.37
#define D5  587.33
#define Eb5 622.25
#define E5  659.26
#define F5  698.46
#define Gb5 739.99
#define G5  783.99
#define Ab5 830.61
#define LA5 880.00
#define Bb5 932.33
#define B5  987.77
#define C6  1046.50
#define Db6 1108.73
#define D6  1174.66
#define Eb6 1244.51
#define E6  1318.51
#define F6  1396.91
#define Gb6 1479.98
#define G6  1567.98
#define Ab6 1661.22
#define LA6 1760.00
#define Bb6 1864.66
#define B6  1975.53
#define C7  2093.00
#define Db7 2217.46
#define D7  2349.32
#define Eb7 2489.02
#define E7  2637.02
#define F7  2793.83
#define Gb7 2959.96
#define G7  3135.96
#define Ab7 3322.44
#define LA7 3520.01
#define Bb7 3729.31
#define B7  3951.07
#define C8  4186.01
#define Db8 4434.92
#define D8  4698.64
#define Eb8 4978.03
// DURATION OF THE NOTES 
#define BPM 120    //  you can change this value changing all the others
#define H 2*Q //half 2/4
#define Q 60000/BPM //quarter 1/4 
#define E Q/2   //eighth 1/8
#define S Q/4 // sixteenth 1/16
#define W 4*Q // whole 4/4



void playGameOverTheme() {
    tone(buzzerPin,LA3,Q); 
    delay(1+Q); //delay duration should always be 1 ms more than the note in order to separate them.
    tone(buzzerPin,LA3,Q);
    delay(1+Q);
    tone(buzzerPin,LA3,Q);
    delay(1+Q);
    tone(buzzerPin,F3,E+S);
    delay(1+E+S);
    tone(buzzerPin,C4,S);
    delay(1+S);
    
    tone(buzzerPin,LA3,Q);
    delay(1+Q);
    tone(buzzerPin,F3,E+S);
    delay(1+E+S);
    tone(buzzerPin,C4,S);
    delay(1+S);
    tone(buzzerPin,LA3,H);
    delay(1+H);
   
}

