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

#ifndef MAIN_HPP
#define MAIN_HPP

#include <stdint.h>


//Persistant parameters
struct user_vars
{
  const int version = 1;
  float farDistance = 200.0;
  float midDistance = 75.0;
  float nearDistance = 40.0;
  float hystDistance = 5.0;
  char upload_user[32] =  "admin" ;
  char upload_pwrd[32] =  "admin" ;
  char MQTT_server[16] =  "192.168.2.103" ;
  char MQTT_client_name[32] = "ESP8266_GARAGE";
  char distance_topic[64] = "sensor/garage/distance";
  char lwt_topic[64] = "sensor/garage/status" ;
  char lwt_status_disconnected[32] =  "disconnected" ;
  char lwt_status_running[32] =  "connected" ;
  uint8_t LEDbrightness = 50;
  int LEDtimeout = 30;
};

#endif  //MAIN_HPP