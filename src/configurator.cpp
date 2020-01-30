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
    Present the user with a menu from 1-8 of options
    9 is for more options if there are more than 8 options
    0 is for back or quit or cancel
    A \r\n from client on a menu resends the menu
    A \r\n from client on configuring an options takes the defualt

    During user input, block waiting on serial data in

    To traverse the menus while not blocking,
        build up a command string that is reset when:
            the end of the menu is reached and executed
            user starts backing out of nested menus
*/

#include <Arduino.h>
#include "HardwareSerial.h"
#include <stdint.h>
#include <string>
#include <EEPROM.h>
#include "main.hpp"
#include "configurator.hpp"

const char newScreen = 12;

const static String mainMenu = {   "\
[1]: Distances\r\n\
[2]: MQTT\r\n\
[3]: LED\r\n\
[4]: Upload\r\n\
[5]: Save\r\n" };

const static String distanceMenu = { "\
[1]: Near\r\n\
[2]: Mid\r\n\
[3]: Far\r\n\
[4]: Sensitivity:\r\n\
[0]: Back\r\n" };

const static String MQTTMenu = { "\
[1]: Server address\r\n\
[2]: Client name\r\n\
[3]: Distance topic\r\n\
[4]: LWT topic\r\n\
[5]: LWT disconnected message\r\n\
[6]: LWT connected message\r\n\
[0]: Back\r\n" };

const static String LEDMenu = { "\
[1]: Brightness\r\n\
[0]: Back\r\n" };

const static String uploadMenu = { "\
[1]: Username\r\n\
[2]: Password\r\n\
[0]: Back\r\n" };

const static String SaveMenu = { "\
[1]: Save values\r\n\
[0]: Back\r\n" };


String command = { "" };


void Configurator::begin()
{
    Serial.begin(115200);
    //Set current page up
    currentPage = Main;
    displayMenu(currentPage);
    //Initialize EEPROM with 512 bytes
    EEPROM.begin(512);
}


void Configurator::service()
{
    while(Serial.available()) {
        char c = Serial.read();
        command += c;
        Serial.print(command);
    }
#if 0
    //Find \r\n
    if((command.indexOf("\r\n") > 0) || (command.indexOf("\r") > 0)) {
        //Look right in front of EoL and execute command
        int selection = (int)command[0] - 48;
        command.clear();
        //Decode what menu we're on and what we should do with the input
        menuDecode(selection);
    }
#endif
    if(command.length() > 0) {
        //Convert from ASCII to int
        int selection = (int)command[0] - 48;
        //Clear the accumulation buffer
        command.clear();
        //Decode what menu to go to from user input
        menuDecode(selection);
    }
}

/*
    Blocking function
*/
String Configurator::getUserInput(String prompt) 
{
    //Clear the screen
    Serial.print(newScreen);
    //Prompt user for input
    Serial.print(prompt);
    //Wait for user input
    String input;
    while((input.indexOf("\r\n") < 0) && (input.indexOf('\r') < 0) && (input.indexOf('\n') < 0)) {
        if(Serial.available()) {
            char c = Serial.read();
            input += c;
            Serial.print(c);
        }
        //yield so the watchdog doesnt kill us
        yield();
    }
    //Remove the EoL
    input.remove(input.indexOf('\n'), 1);
    input.remove(input.indexOf('\r'), 1);

    return (input);
}


void Configurator::displayMenu(enum pageLayout page)
{
    //Clear the terminal
    Serial.print(newScreen);

    //Print the menu option to the terminal
    switch (page)
    {
    case Main:
        Serial.print(mainMenu);
        break;
    case Distance:
        Serial.print(distanceMenu);
        break;
    case Upload:
        Serial.print(uploadMenu);
        break;
    case MQTT:
        Serial.print(MQTTMenu);
        break;
    case LED:
        Serial.print(LEDMenu);
        break;    
    case Save:
        Serial.print(SaveMenu);
        break;
    default:
        Serial.print("displayMenu called with invalid option.\r\n");
        break;
    }
}

void Configurator::menuDecode(int select)
{
    switch (currentPage)
    {
    case Main:
        mainMenuHandler(select);
        break;
    case Distance:
        distanceMenuHandler(select);
        break;
    case Upload:
        uploadMenuHandler(select);
        break;
    case MQTT:
        MQTTMenuHandler(select);
        break;
    case LED:
        LEDMenuHandler(select);
        break;
    case Save:
        SaveMenuHandler(select);
        break;
    
    default:
        break;
    }
}

void Configurator::mainMenuHandler(int select)
{
    //set current page and print to serial
    switch (select)
    {
    case Distance:
        currentPage = Distance;
        break;
    case MQTT:
        currentPage = MQTT;
        break;
    case LED:
        currentPage = LED;
        break;
    case Upload:
        currentPage = Upload;
        break;
    case Save:
        currentPage = Save;
        break;
    
    default:
        break;
    }

    displayMenu(currentPage);
}

void Configurator::distanceMenuHandler(int select)
{
    String result;

    switch(select) {
    //Near
    case 1:
        result = getUserInput("Near distance: ");
        if(result.toFloat() != 0) {
            setNearDistance(result.toFloat());
        }
        else {  //Use the defualt

        }
        break;
    //Mid
    case 2:
        result = getUserInput("Midrange distance: ");
        if(result.toFloat() != 0) {
            setMidDistance(result.toFloat());
        }
        break;
    //Far
    case 3:
        result = getUserInput("Far distance: ");
        if(result.toFloat() != 0) {
            setFarDistance(result.toFloat());
        }
        break;
    //Sensitivity
    case 4:
        result = getUserInput("Sensitivity(cm): ");
        if(result.toFloat() != 0) {
            setHystDistance(result.toFloat());
        }
        break;
    //Back
    case 0:
        currentPage = Main;
        break;
    };

    //Return back to previous menu
    displayMenu(currentPage);
}

void Configurator::uploadMenuHandler(int select)
{
    switch(select){
    case 1: //Username

        break;
    case 2: //Password

        break;
    case 0:
        currentPage = Main;
        break;
    };

    //Return back to previous menu
    displayMenu(currentPage);
}

void Configurator::MQTTMenuHandler(int select)
{
    switch(select){
    case 1: //Server address

        break;
    case 2: //Client name

        break;
    case 3: //Distance topic

        break;
    case 4: //LWT topic

        break;
    case 5: //LWT disconnected message

        break;
    case 6: //LWT connected message

        break;
    case 0:
        currentPage = Main;
        break;
    };

    //Return back to previous menu
    displayMenu(currentPage);
}

void Configurator::LEDMenuHandler(int select)
{
    switch(select){
    case 1: //Brightness

        break;
    case 0:
        currentPage = Main;
        break;
    };

    //Return back to previous menu
    displayMenu(currentPage);
}

void Configurator::SaveMenuHandler(int select)
{
    int bytesWritten = 0;

    switch(select){
    case 1:
        //Save uvars to EEPROM
        bytesWritten = EEPROM_writeAnything(0, uvars);
        Serial.printf("Wrote %u bytes to EEPROM.\r\n", bytesWritten);
        break;
    case 0:
        currentPage = Main;
        break;
    };

    //delay so intermediate text can be read
    delay(2000);

    //Return back to previous menu
    displayMenu(currentPage);
}

void Configurator::setFarDistance(float distance)
{
    uvars.farDistance = distance;
}

void Configurator::setMidDistance(float distance)
{
    uvars.midDistance = distance;
}

void Configurator::setNearDistance(float distance)
{
    uvars.nearDistance = distance;    
}

void Configurator::setHystDistance(float distance)
{
    uvars.hystDistance = distance;
}

void Configurator::setUploadUName(std::string name)
{
    memcpy(uvars.upload_user, &name, name.length());
}

void Configurator::setUploadPWord(std::string word)
{
    memcpy(uvars.upload_pwrd, &word, word.length());
}

void Configurator::setMQTTServer(std::string server)
{
    memcpy(uvars.MQTT_server, &server, server.length());
}

void Configurator::setMQTTClientName(std::string clientName)
{
    memcpy(uvars.MQTT_client_name, &clientName, clientName.length());
}

void Configurator::setDistanceTopic(std::string topic)
{
    memcpy(uvars.distance_topic, &topic, topic.length());
}

void Configurator::setLWTtopic(std::string LWTtopic)
{
    memcpy(uvars.lwt_topic, &LWTtopic, LWTtopic.length());
}

void Configurator::setLWTdisconnectedStatus(std::string dStatus)
{
    memcpy(uvars.lwt_status_disconnected, &dStatus, dStatus.length());
}

void Configurator::setLWTconnectedStatus(std::string cStatus)
{
    memcpy(uvars.lwt_status_running, &cStatus, cStatus.length());
}

void Configurator::setLEDbrightness(uint8_t brightness)
{
    uvars.LEDbrightness = brightness;
}