# HotPotato
IoT project that let you play the "hot potato" game !

Hardware Requirements
-------------
 * 1 x Wemos D1 mini
 * 1 x push button
 * 1 x red LED
 * 1 x buzzer
 * an USB power (either computer or battery)

Software Requirements
-------------
 * EsyMesh Library : https://github.com/Coopdis/easyMesh
 * SimpleList Library : https://github.com/blackhack/ArduLibraries/tree/4b25c1d01a1e4fa05f5a0dfb8678dd29905762e9
 * ArduinoJson : https://github.com/bblanchon/ArduinoJson

Schematics
-------------
![Schematics](./Patate_breadboard.jpg)

Installation
-------------
You'll need the Arduino IDE to compile it for your card.
First, you have to change the players and their node in the setup() function. Then, you may need to change the pin numbers (in the first lines), given than whe have designed this game on a Wemos D1 mini. Finally, change the network SSID and password
