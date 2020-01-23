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

#include <stdint.h>
#include <string>
#include <StateMachine.h>
#include <EEPROM.h>

#include "HardwareSerial.h"
#include "main.hpp"
#include "configStateMachine.hpp"

const char newScreen = 12;

const static String mainMenu = {   "[1]: Distances\r\n \
                                    [2]: MQTT\r\n \
                                    [3]: LED\r\n \
                                    [4]: Upload\r\n" };

const static String distanceMenu = { "[1]: Near\r\n \
                                      [2]: Mid\r\n \
                                      [3]: Far\r\n \
                                      [4]: Sensitivity:\r\n \
                                      [0]: Back\r\n" };

const static String MQTTMenu = { "[1]: Server address\r\n \
                                  [2]: Client name\r\n \
                                  [3]: Distance topic\r\n \
                                  [4]: LWT topic\r\n \
                                  [5]: LWT disconnected message\r\n \
                                  [6]: LWT connected message\r\n \
                                  [0]: Back\r\n" };

const static String LEDMenu = { "[1]: Brightness\r\n \
                                 [0]: Back\r\n" };

const static String uploadMenu = { "[1]: Username\r\n \
                                    [2]: Password\r\n \
                                    [0]: Back" };


ConfigMachine::ConfigMachine() :
    StateMachine(ST_MAX_STATES)
    {
        Serial.begin(115200);
    }

void ConfigMachine::MainReturn(ConfigData* data)
{
    BEGIN_TRANSITION_MAP
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)         //ST_MAIN_MENU
        TRANSITION_MAP_ENTRY (ST_MAIN_MENU)          //ST_DISTANCE
        TRANSITION_MAP_ENTRY (ST_MAIN_MENU)          //ST_UPLOAD
        TRANSITION_MAP_ENTRY (ST_MAIN_MENU)          //ST_MQTT
        TRANSITION_MAP_ENTRY (ST_MAIN_MENU)          //ST_LED
        TRANSITION_MAP_ENTRY (ST_MAIN_MENU)          //ST_USER_INPUT
    END_TRANSITION_MAP(data)
}

void ConfigMachine::DistanceMenu(ConfigData* data)
{
    BEGIN_TRANSITION_MAP
        TRANSITION_MAP_ENTRY (ST_DISTANCE)            //ST_MAIN_MENU
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_DISTANCE
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_UPLOAD
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_MQTT
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_LED
        TRANSITION_MAP_ENTRY (ST_DISTANCE)            //ST_USER_INPUT
    END_TRANSITION_MAP(data)
}

void ConfigMachine::UploadMenu(ConfigData* data)
{
    BEGIN_TRANSITION_MAP
        TRANSITION_MAP_ENTRY (ST_UPLOAD)              //ST_MAIN_MENU
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_DISTANCE
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_UPLOAD
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_MQTT
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_LED
        TRANSITION_MAP_ENTRY (ST_UPLOAD)              //ST_USER_INPUT
    END_TRANSITION_MAP(data)
}

void ConfigMachine::MQTTMenu(ConfigData* data)
{
    BEGIN_TRANSITION_MAP
        TRANSITION_MAP_ENTRY (ST_MQTT)                //ST_MAIN_MENU
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_DISTANCE
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_UPLOAD
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_MQTT
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_LED
        TRANSITION_MAP_ENTRY (ST_MQTT)                //ST_USER_INPUT
    END_TRANSITION_MAP(data)
}

void ConfigMachine::LEDMenu(ConfigData* data)
{
    BEGIN_TRANSITION_MAP
        TRANSITION_MAP_ENTRY (ST_LED)                 //ST_MAIN_MENU
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_DISTANCE
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_UPLOAD
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_MQTT
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)          //ST_LED
        TRANSITION_MAP_ENTRY (ST_LED)                 //ST_USER_INPUT
    END_TRANSITION_MAP(data)
}

STATE_DEFINE(ConfigMachine, MainMenu, ConfigData)
{
    Serial.print(newScreen);
    Serial.println("ConfigMachine::MainMenu");
}

STATE_DEFINE(ConfigMachine, Distance, ConfigData)
{
    Serial.print(newScreen);
    Serial.println("ConfigMachine::DistanceMenu");
}

STATE_DEFINE(ConfigMachine, Upload, ConfigData)
{
    Serial.print(newScreen);
    Serial.println("ConfigMachine::UploadMenu");
}

STATE_DEFINE(ConfigMachine, Mqtt, ConfigData)
{
    Serial.print(newScreen);
    Serial.println("ConfigMachine::MQTTMenu");
}

STATE_DEFINE(ConfigMachine, Led, ConfigData)
{
    Serial.print(newScreen);
    Serial.println("ConfigMachine::LEDMenu");
}

STATE_DEFINE(ConfigMachine, User_Input, ConfigData)
{
    Serial.print(newScreen);
    Serial.println("ConfgiMachine::UserInput");
}