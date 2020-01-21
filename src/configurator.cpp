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
*/

#include "HardwareSerial.h"
#include <stdint.h>
#include <string>
#include "main.hpp"
#include "configurator.hpp"

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
    while(Serial.peek() > 0) {
        command += Serial.read();
    }

    //Find \r\n
    if(command.indexOf("\r\n") > 0) {
        //Look right in front of EoL and execute command
        int selection = (int)command[0] - 48;
        command.clear();
        //Decode what menu we're one and what we should do with the input
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
            input += Serial.read();
        }

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
    switch (select)
    {
    case Distance:
        distanceMenuHandler(select);
        break;
    case MQTT:
        MQTTMenuHandler(select);
        break;
    case LED:
        LEDMenuHandler(select);
        break;
    case Upload:
        uploadMenuHandler(select);
        break;
    
    default:
        break;
    }
}

void Configurator::distanceMenuHandler(int select)
{
    switch(select) {
    //Near
    case 1:


        break;
    //Mid
    case 2:

        break;
    //Far
    case 3:

        break;
    //Sensitivity
    case 4:

        break;
    //Back
    case 0:
        currentPage = Main;
        break;
    };
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
}

void Configurator::LEDMenuHandler(int select)
{
    switch(select){
    case 1:

        break;
    case 0:
        currentPage = Main;
        break;
    };
}

void Configurator::setFarDistance(float distance)
{

}


void Configurator::setMidDistance(float distance)
{

}

void Configurator::setNearDistance(float distance)
{

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