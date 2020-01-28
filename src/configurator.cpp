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
#include "main.hpp"
#include "configurator.hpp"

const char newScreen = 12;

const static String mainMenu = {   "\r\n\
[1]: Distances\r\n\
[2]: MQTT\r\n\
[3]: LED\r\n\
[4]: Upload\r\n" };

const static String distanceMenu = { "\r\n\
[1]: Near\r\n\
[2]: Mid\r\n\
[3]: Far\r\n\
[4]: Sensitivity:\r\n\
[0]: Back\r\n" };

const static String MQTTMenu = { "\r\n\
[1]: Server address\r\n\
[2]: Client name\r\n\
[3]: Distance topic\r\n\
[4]: LWT topic\r\n\
[5]: LWT disconnected message\r\n\
[6]: LWT connected message\r\n\
[0]: Back\r\n" };

const static String LEDMenu = { "\r\n\
[1]: Brightness\r\n\
[0]: Back\r\n" };

const static String uploadMenu = { "\r\n\
[1]: Username\r\n\
[2]: Password\r\n\
[0]: Back" };


String command = { "" };


void Configurator::begin()
{
    Serial.begin(115200);
    //Set current page up
    currentPage = Main;
    displayMenu(currentPage);
}


void Configurator::service()
{
    while(Serial.available()) {
        char c = Serial.read();
        command += c;
    }

    //Find \r\n
    if((command.indexOf("\r\n") > 0) || (command.indexOf("\n\r") > 0)) {
        //Look right in front of EoL and execute command
        int selection = (int)command[0] - 48;
        command.clear();
        //Decode what menu we're on and what we should do with the input
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
    while(input.indexOf("\r\n") < 0) {
        if(Serial.available()) {
            char c = Serial.read();
            input += c;
        }
        //yield so the watchdog doesnt kill us
        yield();
    }
    //Remove the EoL
    input.remove(input.length() - 2, 2);

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
        else {

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
    case 1:

        break;
    case 2:

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
    case 1:

        break;
    case 2:

        break;
    case 3:

        break;
    case 4:

        break;
    case 5:

        break;
    case 6:

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

void Configurator::setFarDistance(float distance)
{

}


void Configurator::setMidDistance(float distance)
{

}

void Configurator::setNearDistance(float distance)
{
    Serial.printf("\r\nConfigurator::setNearDistance\r\nSet to %2.2f.\r\n", distance);
}

void Configurator::setHystDistance(float distance)
{

}

void Configurator::setUploadUName(std::string name)
{

}

void Configurator::setUploadPWord(std::string word)
{

}

void Configurator::setMQTTServer(std::string server)
{

}

void Configurator::setMQTTClientName(std::string clientName)
{

}

void Configurator::setDistanceTopic(std::string topic)
{

}

void Configurator::setLWTtopic(std::string LWTtopic)
{

}

void Configurator::setLWTdisconnectedStatus(std::string dStatus)
{

}

void Configurator::setLWTconnectedStatus(std::string cStatus)
{

}

void Configurator::setLEDbrightness(uint8_t brightness)
{

}