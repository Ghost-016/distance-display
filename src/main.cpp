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

//Private librarys
#include <LEDring.hpp>


//===================================
//  Defines/Constants
//===================================
#define LED_ENABLED  1
#define WIFI_ENABLED 0
#define MQTT_ENABLED  (0 & WIFI_ENABLED)


const int BAUD_SERIAL = 115200;

#if MQTT_ENABLED
const char mqtt_server[] = { "192.168.2.103" };

const char MQTT_CLIENT_NAME[] = { "ESP8266_GARAGE" };

const char distance_topic[] = { "sensor/garage/distance" };
const char lwt_topic[] =      { "sensor/garage/status" };
#endif

const int ULTRASONIC_TRIGGER = D3;
const int ULTRASONIC_ECHO = D2;
const int NEOPIXEL_PIN = D1;
const int NUMPIXELS = 16;

const float DIST_HYS = 5.0;
const int LEDTIMEOUT = 30;


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
//distance bounds (to be made configurable at runtime)
const float farDistance = 200.0;
const float midDistance = 75.0;
const float nearDistance = 40.0;
const float hystDistance = 5.0;

//Persistant parameters
struct user_vars
{
  float farDistance;
  float midDistance;
  float nearDistance;
  float hystDistance;
  char upload_user[32];
  char upload_pwrd[32];
  char MQTT_server[16];
  char lwt_topic[64];
  char lwt_status[32];
  uint8_t LEDbrightness;
};



//===================================
//  Class objects
//===================================
#if MQTT_ENABLED
WiFiClient espClient;
PubSubClient client(espClient);
#endif

LEDring ring(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Initialize sensor (trigger, echo)
UltraSonicDistanceSensor distanceSensor(ULTRASONIC_TRIGGER, ULTRASONIC_ECHO);

#if WIFI_ENABLED
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -21600, 60000); //CST offset
#endif

os_timer_t SysTick; //Technically just a type but...its special


//===================================
//  Prototypes
//===================================
bool checkBound(double newVal, double prevVal, double maxDiff);

#if WIFI_ENABLED
void setup_wifi();
#endif

#if MQTT_ENABLED
void reconnect();
#endif

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

  //Start EEPROM with 512 bytes
  EEPROM.begin(512);

  //NeoPixel init
  ring.begin();

#if WIFI_ENABLED
  setup_wifi();
#endif

#if MQTT_ENABLED
  client.setServer(mqtt_server, 1883);
#endif

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
#endif


#if LED_ENABLED
 //Check if its time to update LED array 10Hz
  if (updateLED == true) {
    updateLED = false;
    //Update distance measurment, will delay for quite a while if sensor is not connected
    distance = distanceSensor.measureDistanceCm();

    if(!isnan(distance) && (LEDenabled == true)) {
      //Call LED function
      if ((distance > farDistance) || (distance < 0)) {
        ring.setGreen();
      }
      else if ((distance < midDistance) && (distance > (nearDistance + hystDistance))) {
        ring.setYellow();
      }
      else if ((distance < nearDistance) && (distance > 0.0)) {
        ring.setRed();
      }
    }
    else {
      ring.off();
    }
    //update LEDs regularly
    ring.update();
  }
#endif

  //Check if its time to publish MQTT 1Hz
  //Also check if the LEDs can be turned off
  if (pubMQTT == true) {
    pubMQTT = false;

    if (checkBound(distance, prevdistance, DIST_HYS)) {
      prevdistance = distance;
#if MQTT_ENABLED
      if(client.connected()) {
        client.publish(distance_topic, String(distance).c_str(), true);
      }
#endif
      LEDenabled = true;
    }
    else {
#if WIFI_ENABLED
      //Check if the distance has changed a reasonable ammount in the last 30 seconds
      if(timeClient.getEpochTime() - lastEpochTime >= LEDTIMEOUT) {
        lastEpochTime = timeClient.getEpochTime();
        //disable LEDs
        LEDenabled = false;
      }
#endif
    }
#if WIFI_ENABLED
    //Update time client
    timeClient.update();    
#endif
  }
  //Yeild so WiFi core can process
  yield();
} //Loop()


//Returns true if newVal is greater or less than prevVal by maxDiff
bool checkBound(double newVal, double prevVal, double maxDiff) {
  return (!isnan(newVal) && ((newVal < prevVal - maxDiff) || (newVal > prevVal + maxDiff)));
}
#endif

#if WIFI_ENABLED
void setup_wifi()
{
  //Straight from the basic library example
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  //NTP client
  timeClient.begin();
}
#endif

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
      if (client.connect(MQTT_CLIENT_NAME, lwt_topic, 2, true, "disconnected")) {
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
  client.publish(lwt_topic, "connected");
}
#endif