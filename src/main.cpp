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
  TODO: Add runtime configuration for MQTT server and topics
  TODO: Add runtime configuration for HTTP updater username and password
  TODO: Add runtime configuration for distances
  TODO: Add runtime configuration for LED brightness
  TODOL Add runtime configuration for LED timeout
  TODO: Add runtime option to reset wifi
*/
#ifndef UNIT_TEST

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>
#include <HCSR04.h>
#include <PubSubClient.h>
#include <NTPClient.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>

//Private librarys
#include <LEDring.hpp>

#include "main.hpp"
#include "configurator.hpp"


//===================================
//  Configuration
//===================================

#define LED_ENABLED  1
#define WIFI_ENABLED 0
#define MQTT_ENABLED  (0 & WIFI_ENABLED)


//===================================
//  Constants
//===================================

const int BAUD_SERIAL = 115200;

const int ULTRASONIC_TRIGGER = D3;
const int ULTRASONIC_ECHO = D2;
const int NEOPIXEL_PIN = D1;
const int NUMPIXELS = 16;

const float DIST_HYS = 5.0;
const int LEDTIMEOUT = 30;

//Web updater
const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";


//===================================
//  Global Variables
//===================================
volatile bool serialDistance = false;
volatile bool updateLED = false;
volatile bool pubMQTT = false;
volatile int tick = 0;

bool LEDenabled = true;

long lastEpochTime = 0;

//distance
float distance = 0.00;
float prevdistance = 0.00;

//struct user_vars uvars;



//===================================
//  Class objects
//===================================
#if MQTT_ENABLED
WiFiClient espClient;
PubSubClient client(espClient);
#endif  //#if MQTT_ENABLED

LEDring ring(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Initialize sensor (trigger, echo)
UltraSonicDistanceSensor distanceSensor(ULTRASONIC_TRIGGER, ULTRASONIC_ECHO);

#if WIFI_ENABLED
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -21600, 60000); //CST offset
#endif  //#if WIFI_ENABLED

os_timer_t SysTick; //Technically just a type but...its special

//Web updater
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//Config
Configurator config;


//===================================
//  Prototypes
//===================================
bool checkBound(float newVal, float prevVal, float maxDiff);

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
  //Start EEPROM with 512 bytes
  EEPROM.begin(512);

  //Start configurator, uses Serial
  config.begin();

  //NeoPixel init
  ring.begin();

#if WIFI_ENABLED
  setup_wifi();
#endif  //#if WIFI_ENABLED

#if MQTT_ENABLED
  client.setServer(uvars.MQTT_server, 1883);
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


#if LED_ENABLED
 //Check if its time to update LED array 10Hz
  if (updateLED == true) {
    updateLED = false;
    //Update distance measurment, will delay for quite a while if sensor is not connected
    distance = distanceSensor.measureDistanceCm();

    if(!isnan(distance) && (LEDenabled == true)) {
      //Call LED function
      if ((distance > config.uvars.farDistance) || (distance < 0)) {
        ring.setGreen();
      }
      else if ((distance < config.uvars.midDistance) && (distance > (config.uvars.nearDistance + config.uvars.hystDistance))) {
        ring.setYellow();
      }
      else if ((distance < config.uvars.nearDistance) && (distance > 0.0)) {
        ring.setRed();
      }
    }
    else {
      ring.off();
    }
    //update LEDs regularly
    ring.update();

    //service the configurator at the same rate as the LEDs
    config.service();
  }
#endif  //#if LED_ENABLED

  //Check if its time to publish MQTT 1Hz
  //Also check if the LEDs can be turned off
  if (pubMQTT == true) {
    pubMQTT = false;

    if (checkBound(distance, prevdistance, DIST_HYS)) {
      prevdistance = distance;
#if MQTT_ENABLED
      if(client.connected()) {
        client.publish(uvars.distance_topic, String(distance).c_str(), true);
      }
#endif  //#if MQTT_ENABLED
      LEDenabled = true;
#if WIFI_ENABLED
      //update lastEpochTime to reset the timeout
      lastEpochTime = timeClient.getEpochTime();
#endif  //#if WIFI_ENABLED
    }
    else {
#if WIFI_ENABLED
      //Check if the distance has changed a reasonable ammount in the last 30 seconds
      if(timeClient.getEpochTime() - lastEpochTime >= LEDTIMEOUT) {
        lastEpochTime = timeClient.getEpochTime();
        //disable LEDs
        LEDenabled = false;
      }
#endif  //#if WIFI_ENABLED
    }
#if WIFI_ENABLED
    //Update time client
    timeClient.update();    
#endif  //#if WIFI_ENABLED
  }
  //Yeild so WiFi core can process
  yield();
} //Loop()


//Returns true if newVal is greater or less than prevVal by maxDiff
bool checkBound(float newVal, float prevVal, float maxDiff) {
  return (!isnan(newVal) && ((newVal < prevVal - maxDiff) || (newVal > prevVal + maxDiff)));
}
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

  httpUpdater.setup(&httpServer, update_path, uvars.upload_user, uvars.upload_pwrd);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
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
      if (client.connect(uvars.MQTT_client_name, uvars.lwt_topic, 2, true, uvars.lwt_status_disconnected)) {
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
  client.publish(uvars.lwt_topic, uvars.lwt_status_running);
}
#endif  //#if MQTT_ENABLED

bool EEPROM_valid()
{
  if(EEPROM.read(0) == 0xFF) {
    return false;
  }
  else {
    return true;
  }
}