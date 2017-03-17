# HotPotato
IoT project that let you play the "hot potato" game !

Game rules
-------------
 
First, the players choose who will have the potato on their arduino webpage.
There must be one player with a potato. The others will not have one.
As soon as the potato is created a timer, invisible to the players, is set between 10 and 120 secondes.
A red light tells to the player that he have the potato.
Then, the player with a potato can push the button to send it to another player.
When the potato explodes (when the timer reach 0) the buzzer makes a sound.
The player who had it at this moment loose the game.
Finally, The scores are updates on the webpage.
 
 
Hardware Requirements
-------------
 * 1 x Wemos D1 mini
 * 1 x push button
 * 1 x red LED
 * 1 x buzzer
 * an micro USB power (either computer or battery)

Software Requirements
-------------
 * EasyMesh Library : https://github.com/Coopdis/easyMesh
 * SimpleList Library : https://github.com/blackhack/ArduLibraries/tree/4b25c1d01a1e4fa05f5a0dfb8678dd29905762e9
 * ArduinoJson : https://github.com/bblanchon/ArduinoJson

Schematics
-------------
![Schematics](./Patate_breadboard.jpg)

Installation
-------------
You'll need the Arduino IDE to compile it for your card.
First, you have to change the players and their node in the setup() function. Then, you may need to change the pin numbers (in the first lines), given than whe have designed this game on a Wemos D1 mini. Finally, change the network SSID and password
