# alexa-Music-Lamp
Sound reactive led lamp using esp8266(NODE MCU). which can be controlled via alexa


## Hardware Used
* NODEMCU(esp8266)
* ws2812B leds
* arduino microphone module

## Circuit Connection

* Connect Nodemcu vin to led strip 5v input
* Connect Nodemcu GND to led strip GND input
* Connect Nodemcu D4 to led strip data input

* Connect Nodemcu 3.3v to microphone VCC pin
* Connect Nodemcu GND to led microphone GND pin
* Connect Nodemcu A0 to microphone analog output A0

## Software
### prerequisite
  * Arduino IDE
  * Follow this [guide|https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/] for Installing ESP8266 Board in Arduino IDE
  * Install FastLED library  (Tools->Manage Libraries.search for FastLED)
  * Espalexa library (Tools->Manage Libraries.search for Espalexa)
 
 ### Code
 * Download `esp8266_alexa_lamp.ino` and `reactive_common.h` 
 * Change Wifi ssid/password(variables `WIFI_SSID` and `WIFI_PASSWORD`), led type and no of led(variables `NUMLEDS`, `LED_TYPE`, `COLOR_ORDER`)
 * Connect and Upload to NODEMCU.

## Alexa
* Open Alexa app and search for new devices(type `other`) and add them.
* Commands supported are `Alexa lamp On/Off`, `Alexa lamp colour <colour>` , `Alexa lamp brightness 0-100%` , `Alexa disco On/Off`
