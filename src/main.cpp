/*
MIT License

Copyright (c) 2020 Ghost-016

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


/*
  TODO: Add runtime configuration for LED timeout
  TODO: Add runtime option to reset wifi

  Experiment with making each feature its own class for code readability and reuse.
*/
#ifndef UNIT_TEST

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include <HCSR04.h>

#include <PubSubClient.h>
#include <NTPClient.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>

//Conflicts with ESP8266WebServer.h
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>

#include "configurator.hpp"
#include "display.hpp"

//===================================
//  Constants
//===================================

const int BAUD_SERIAL = 115200;

//Web updater
const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";

//HCSR04 ultrasonic distance sensor connection
const int ULTRASONIC_TRIGGER = D3;
const int ULTRASONIC_ECHO = D2;


//===================================
//  Global Variables
//===================================
volatile bool updateLED = false;
volatile bool pubMQTT = false;
volatile int tick = 0;

bool LEDenabled = true;

float distance;

//===================================
//  Class objects
//===================================

//WiFi client stuffs
WiFiClient espClient;

//MQTT client stuffs
PubSubClient client(espClient);

//NTP client stuffs
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -21600, 60000); //CST offset

os_timer_t SysTick; //Technically just a type but...its special

//Web updater
ESP8266WebServer httpServer(8080);
ESP8266HTTPUpdateServer httpUpdater;

//Web configuration
ESP8266WebServer configServer;

//Override data in/out for configurator class to serial
class userConfig : public Configurator
{
  public:
  virtual void dataOut(String output)
  {
    Serial.write(output.c_str());
  }

  virtual char dataIn(void)
  {
    if(Serial.available()) {
      char c;
      c = Serial.read();
      return (c);
    }
    else {
      return (0);
    }
  }
};

userConfig config;

display *LEDdisplay;
UltraSonicDistanceSensor *distanceSensor;


//===================================
//  Prototypes
//===================================

void setup_wifi();

void reconnect();

void handleRoot();
void drawGraph();
void handleNotFound();

//===================================
//  SysTick ISR
//===================================
void SysTickHandler(void *pArg)
{
  //update the LEDs every time timer fires
  updateLED = true;

  //publish MQTT from distance at 1Hz
  if ((tick % 10) == 0) {
    pubMQTT = true;
  }

  tick++;

  if (tick > 20) tick = 1;
}


//===================================
//  Setup
//===================================
void setup() {
  //Initialize serial for configuration
  Serial.begin(BAUD_SERIAL);

  //Start configurator, uses Serial
  config.begin();

  //Initialize sensor (trigger, echo)
  distanceSensor = new UltraSonicDistanceSensor(ULTRASONIC_TRIGGER, ULTRASONIC_ECHO);

  //Initialize LED ring display
  LEDdisplay = new display;

  //NeoPixel init
  LEDdisplay->begin(config, timeClient);

  //Connect to wifi, blocks until wifi established
  setup_wifi();

#if 0
  //MQTT client setup
  client.setServer(config.getMQTTServer().c_str(), 1883);
#endif

  //SysTick init
  os_timer_setfn(&SysTick, &SysTickHandler, NULL);

  //Enable SysTick
  os_timer_arm(&SysTick, 100, true);

    configServer.on("/", handleRoot);
    configServer.on("/test.svg", drawGraph);
    configServer.onNotFound(handleNotFound);
    configServer.begin();
}

//===================================
//  Main Loop
//===================================
void loop() {
  #if 0
  //MQTT client
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  #endif
  //HTTP update server
  httpServer.handleClient();
  //Multicast DNS to respond to our URL
  MDNS.update();

  configServer.handleClient();


 //Check if its time to update LED array 10Hz
  if (updateLED == true) {
    updateLED = false;

    //Update distance measurment, will delay for quite a while if sensor is not connected
    distance = distanceSensor->measureDistanceCm();

    //Service the LED display
    LEDdisplay->loop(distance);

    //service the configurator at the same rate as the LEDs
    while(Serial.available()) {
      //Send data to configurator service
      config.service(Serial.read());
    }
  }


  //Check if its time to publish MQTT 1Hz
  //Also check if the LEDs can be turned off
  if (pubMQTT == true) {
    pubMQTT = false;
    if(client.connected()) {
      client.publish(config.getDistanceTopic().c_str(), String(distance).c_str(), true);
    } 
    //Update time client
    timeClient.update();

  }
  
  //Yeild so WiFi core can process
  yield();
} //loop()


void setup_wifi()
{
  //Straight from the basic library example
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  //NTP client
  timeClient.begin();
  //Set the last time to when we start so LEDs will go for ~max timout time
  timeClient.update();

  //Web updater server
  MDNS.begin(host);

  httpUpdater.setup(&httpServer, update_path, config.getUploadUsername(), config.getUploadPassword());
  //To avoid conflict with the configuration web server, set port to 8080
  httpServer.begin(8080);

  MDNS.addService("http", "tcp", 8080);
}

void reconnect()
{
  static long lastReconnectAttempt = 0;
  //Keep trying until we're reconnected but dont hold up the rest of the program
  if(!client.connected()) {
    long now = millis();
    //If its been 5 seconds
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      //Attempt to connect and send LWT topic and message
      if (client.connect(config.getMQTTClientName().c_str(), config.getLWTTopic().c_str(), 2, true, config.getLWTDisconnected().c_str())) {
        lastReconnectAttempt = 0;
      }
    }
  }

  if(client.connected()) {
    //Connected, update LWT topic
    client.publish(config.getLWTTopic().c_str(), config.getLWTConnected().c_str());
  }
}

void handleRoot()
{
    char temp[400];
    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;

    snprintf(temp, 400,
               "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60
    );

    configServer.send(200, "text/html", temp);
}

void drawGraph() {
  String out;
  out.reserve(2600);
  char temp[70];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  configServer.send(200, "image/svg+xml", out);
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += configServer.uri();
    message += "\nMethod: ";
    message += (configServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += configServer.args();
    message += "\n";

    for (uint8_t i = 0; i < configServer.args(); i++) {
        message += " " + configServer.argName(i) + ": " + configServer.arg(i) + "\n";
    }

    configServer.send(404, "text/plain", message);
}

#endif  //#ifdef UNIT_TEST