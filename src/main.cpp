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

#include "main.hpp"
#include "configurator.hpp"


//===================================
//  Configuration
//===================================

#define WIFI_ENABLED 1
#define MQTT_ENABLED  (1 & WIFI_ENABLED)



//===================================
//  Constants
//===================================

const int BAUD_SERIAL = 115200;

//Web updater
const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";

const int ULTRASONIC_TRIGGER = D3;
const int ULTRASONIC_ECHO = D2;


//===================================
//  Global Variables
//===================================
volatile bool serialDistance = false;
volatile bool updateLED = false;
volatile bool pubMQTT = false;
volatile int tick = 0;

bool LEDenabled = true;

long lastEpochTime = 0;

float distance;

//===================================
//  Class objects
//===================================
#if MQTT_ENABLED
WiFiClient espClient;
PubSubClient client(espClient);
#endif  //#if MQTT_ENABLED

#if WIFI_ENABLED
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -21600, 60000); //CST offset
#endif  //#if WIFI_ENABLED

os_timer_t SysTick; //Technically just a type but...its special

//Web updater
ESP8266WebServer httpServer(8080);
ESP8266HTTPUpdateServer httpUpdater;

//Config
//Configurator config;

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

#if WIFI_ENABLED
void setup_wifi();
#endif  //#if WIFI_ENALBED

#if MQTT_ENABLED
void reconnect();
#endif  //#if MQTT_ENABLED

//===================================
//  SysTick ISR
//===================================
void SysTickHandler(void *pArg)
{
  //update the LEDs every time timer fires
  updateLED = true;

  //publish distance to serial at 2Hz
  if ((tick % 5) == 0) {
    serialDistance = true;
  }

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
  Serial.begin(BAUD_SERIAL);

  //Start configurator, uses Serial
  config.begin();

  //Initialize sensor (trigger, echo)
  distanceSensor = new UltraSonicDistanceSensor(ULTRASONIC_TRIGGER, ULTRASONIC_ECHO);

  LEDdisplay = new display;

  //NeoPixel init
  LEDdisplay->begin(&config, &timeClient);

#if WIFI_ENABLED
  setup_wifi();
#endif  //#if WIFI_ENABLED

#if MQTT_ENABLED
  client.setServer(config.getMQTTServer().c_str(), 1883);
#endif  //#if MQTT_ENABLED

  //SysTick init
  os_timer_setfn(&SysTick, &SysTickHandler, NULL);

  //Enable SysTick
  os_timer_arm(&SysTick, 100, true);
}

//===================================
//  Main Loop
//===================================
void loop() {

#if MQTT_ENABLED
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
#endif //#if MQTT_ENABLED

#if WIFI_ENABLED
  httpServer.handleClient();
  MDNS.update();
#endif //#if WIFI_ENABLED


 //Check if its time to update LED array 10Hz
  if (updateLED == true) {
    updateLED = false;

    //Update distance measurment, will delay for quite a while if sensor is not connected
    distance = distanceSensor->measureDistanceCm();

    LEDdisplay->loop(distance);

    //service the configurator at the same rate as the LEDs
    while(Serial.available()) {
      //Send serial data to configurator service
      config.service(Serial.read());
    }
  }


  //Check if its time to publish MQTT 1Hz
  //Also check if the LEDs can be turned off
  if (pubMQTT == true) {
    pubMQTT = false;

#if MQTT_ENABLED
    if(client.connected()) {
      client.publish(config.getDistanceTopic().c_str(), String(distance).c_str(), true);
    } 
#endif

#if WIFI_ENABLED
    //Update time client
    timeClient.update();    
#endif  //#if WIFI_ENABLED

  }
  
  //Yeild so WiFi core can process
  yield();
} //loop()


#endif  //#ifdef UNIT_TEST

#if WIFI_ENABLED
void setup_wifi()
{
  //Straight from the basic library example
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  //NTP client
  timeClient.begin();
  //Set the last time to when we start so LEDs will go for ~max timout time
  timeClient.update();
  lastEpochTime = timeClient.getEpochTime();

  //Web updater server
  MDNS.begin(host);

  httpUpdater.setup(&httpServer, update_path, config.getUploadUsername(), config.getUploadPassword());
  //To avoid conflict with the configuration web server, set port to 8080
  httpServer.begin(8080);

  MDNS.addService("http", "tcp", 8080);
}
#endif  //#if WIFI_ENABLED

#if MQTT_ENABLED
void reconnect()
{
  static long lastReconnectAttempt = 0;
  //Keep trying until we're reconnected but dont hold up the rest of the program
  if(!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      //Attempt to connect and send LWT topic and message
      if (client.connect(config.getMQTTClientName().c_str(), config.getLWTTopic().c_str(), 2, true, config.getLWTDisconnected().c_str())) {
        //Serial.println("connected");
        lastReconnectAttempt = 0;
      }
      else {
        //Serial.print("failed, rc=");
        //Serial.print(client.state());
        //Serial.println(" trying again in 5 seconds");
        //Wait 5 seconds before retrying
        //delay(5000);
      }
    }
  }
  //Connected, update LWT topic
  client.publish(config.getLWTTopic().c_str(), config.getLWTConnected().c_str());
}
#endif  //#if MQTT_ENABLED
