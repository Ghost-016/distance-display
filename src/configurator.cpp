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

    TODO: Make this independant of Serial
    TODO: Use String instead of std::string
*/

#include <Arduino.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <EEPROM.h>
#include "configurator.hpp"

const static String newScreen = {"\f"};

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
[2]: Timeout\r\n\
[0]: Back\r\n" };

const static String uploadMenu = { "\
[1]: Username\r\n\
[2]: Password\r\n\
[0]: Back\r\n" };

const static String SaveMenu = { "\
[1]: Save values\r\n\
[2]: Dump EEPROM\r\n\
[0]: Back\r\n" };


void Configurator::begin()
{
    //Make sure input is blanked
    command = {""};

    //Set current page up
    currentPage = Main;
    displayMenu(currentPage);

    //Initialize EEPROM with 512 bytes
    EEPROM.begin(512);
    //Read from EEPROM and verify if the data is valid
    if(readEEPROMversion()) {
        EEPROM_readAnything(0, uvars);
    }
}


void Configurator::service(char input)
{
    //Accumulate input from user
    command += input;

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
String Configurator::getUserInput(String prompt, String defVal) 
{
    //Clear the screen
    dataOut(newScreen);

    //Prompt user for input
    char buf[128];
    sprintf(buf, "%s (%s): ", prompt.c_str(), defVal.c_str());
    //outputFunc(buf);
    dataOut(buf);

    //Wait for user input
    String input = {""};
    while((input.indexOf("\r\n") < 0) && (input.indexOf('\r') < 0) && (input.indexOf('\n') < 0)) {
        char c = dataIn();
        if(c != 0) {
            dataOut(String(c));
            input += c;
        }

        //yield so the watchdog doesnt kill us
        yield();
    }
    //Remove the EoL
    input.remove(input.indexOf('\n'), 1);
    input.remove(input.indexOf('\r'), 1);

    return (input);
}

/*
    Blocking function
*/
String Configurator::getUserInput(String prompt, float defVal) 
{
    //Clear the screen
    dataOut(newScreen);

    //Prompt user for input
    char buf[128];
    sprintf(buf, "%s (%3.2f): ", prompt.c_str(), defVal);
    //outputFunc(buf);
    dataOut(buf);

    //Wait for user input, with timeout?
    String input = {""};
    while((input.indexOf("\r\n") < 0) && (input.indexOf('\r') < 0) && (input.indexOf('\n') < 0)) {
        char c = dataIn();
        if(c != 0) {
            dataOut(String(c));
            input += c;
        }
        
        //yield so the watchdog doesnt kill us
        yield();
    } 
    //Remove the EoL
    input.remove(input.indexOf('\n'), 1);
    input.remove(input.indexOf('\r'), 1);

    return (input);
}

/*
    Blocking function
*/
String Configurator::getUserInput(String prompt, int defVal) 
{
    //Clear the screen
    dataOut(newScreen);

    //Prompt user for input
    char buf[128];
    sprintf(buf, "%s (%u): ", prompt.c_str(), defVal);
    //outputFunc(buf);
    dataOut(buf);

    //Wait for user input
    String input = {""};
    while((input.indexOf("\r\n") < 0) && (input.indexOf('\r') < 0) && (input.indexOf('\n') < 0)) {
        char c = dataIn();
        if(c != 0) {
            dataOut(String(c));
            input += c;
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
    dataOut(newScreen);

    //Print the menu option to the terminal
    switch (page)
    {
    case Main:
        dataOut(mainMenu);
        break;
    case Distance:
        dataOut(distanceMenu);
        break;
    case Upload:
        dataOut(uploadMenu);
        break;
    case MQTT:
        dataOut(MQTTMenu);
        break;
    case LED:
        dataOut(LEDMenu);
        break;    
    case Save:
        dataOut(SaveMenu);
        break;
    default:
        dataOut("displayMenu called with invalid option.\r\n");
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
    String defVal;

    switch(select) {
    //Near
    case 1:
        result = getUserInput("Near distance", uvars.nearDistance);
        if(result.toFloat() != 0) {
            setNearDistance(result.toFloat());
        }
        else {  //Use the defualt

        }
        break;
    //Mid
    case 2:
        result = getUserInput("Midrange distance", uvars.midDistance);
        if(result.toFloat() != 0) {
            setMidDistance(result.toFloat());
        }
        break;
    //Far
    case 3:
        result = getUserInput("Far distance", uvars.farDistance);
        if(result.toFloat() != 0) {
            setFarDistance(result.toFloat());
        }
        break;
    //Sensitivity
    case 4:
        result = getUserInput("Sensitivity(cm)", uvars.hystDistance);
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
    String result = {""};
    switch(select){
    case 1: //Username
        result = getUserInput("Upload username", uvars.upload_user);
        if(result.length() > 0) {
            setUploadUName(result.c_str());
        }
        break;
    case 2: //Password
        result = getUserInput("Upload password", uvars.upload_pwrd);
        if(result.length() > 0) {
            setUploadPWord(result.c_str());
        }
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
    String result = {""};

    switch(select){
    case 1: //Server address
        result = getUserInput("MQTT Server IP address", uvars.MQTT_server);
        if(result.length() > 0) {
            setMQTTServer(result.c_str());
        }
        break;
    case 2: //Client name
        result = getUserInput("MQTT Client name", uvars.MQTT_client_name);
        if(result.length() > 0) {
            setMQTTClientName(result.c_str());
        }        
        break;
    case 3: //Distance topic
        result = getUserInput("MQTT Distance topic", uvars.distance_topic);
        if(result.length() > 0) {
            setDistanceTopic(result.c_str());
        } 
        break;
    case 4: //LWT topic
        result = getUserInput("MQTT Last Will topic", uvars.lwt_topic);
        if(result.length() > 0) {
            setLWTtopic(result.c_str());
        } 
        break;
    case 5: //LWT disconnected message
        result = getUserInput("MQTT Last Will disconnected message", uvars.lwt_status_disconnected);
        if(result.length() > 0) {
            setLWTdisconnectedStatus(result.c_str());
        } 
        break;
    case 6: //LWT connected message
        result = getUserInput("MQTT Last Will connected message", uvars.lwt_status_running);
        if(result.length() > 0) {
            setLWTconnectedStatus(result.c_str());
        } 
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
    String result = {""};
    String defVal;

    switch(select){
    case 1: //Brightness
        result = getUserInput("LED brightness (1-255)", (int)uvars.LEDbrightness);
        if(result.toFloat() > 0) {
            setLEDbrightness(result.toFloat());
        }
        break;
    case 2: //Timeout
        result = getUserInput("LED Timeout (s)", uvars.LEDtimeout);
        if(result.toFloat() > 0) {
            //setLEDbrightness(result.toInt());
        }
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
        EEPROM.commit();
        //Report bytes written to EEPROM
        char c[64];
        sprintf((char *)c, "Wrote %u bytes to EEPROM.\r\n", bytesWritten);
        dataOut(String(c));
        break;
    case 2:
        dumpEEPROM();
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

void Configurator::dumpEEPROM()
{
    struct user_vars test_vars;
    EEPROM_readAnything(0, test_vars);
    char c[1024];
    sprintf((char *)c, "\
    version: %u\r\n\
    farDistance: %3.2f\r\n\
    midDistance: %3.2f\r\n\
    nearDistance: %3.2f\r\n\
    hystDistance: %3.2f\r\n\
    upload_user: %s\r\n\
    upload_pwrd: %s\r\n\
    MQTT_server: %s\r\n\
    MQTT_client_name: %s\r\n\
    distance_topic: %s\r\n\
    lwt_topic: %s\r\n\
    lwt_status_disconnected: %s\r\n\
    lwt_status_running: %s\r\n\
    LEDbrightness: %u\r\n\
    LEDtimeout: %u\r\n",
    test_vars.version, test_vars.farDistance, test_vars.midDistance, test_vars.nearDistance, test_vars.hystDistance, test_vars.upload_user, test_vars.upload_pwrd,
    test_vars.MQTT_server, test_vars.MQTT_client_name, test_vars.distance_topic, test_vars.lwt_topic, test_vars.lwt_status_disconnected, test_vars.lwt_status_running,
    test_vars.LEDbrightness, test_vars.LEDtimeout);
    dataOut(String(c));
}

bool Configurator::readEEPROMversion()
{
    int eepromVersion;
    EEPROM_readAnything(0, eepromVersion);
    if(eepromVersion != 1)
        return false;
    else
        return true;
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

void Configurator::setUploadUName(String name)
{
    memcpy(uvars.upload_user, &name, name.length());
}

void Configurator::setUploadPWord(String word)
{
    memcpy(uvars.upload_pwrd, &word, word.length());
}

void Configurator::setMQTTServer(String server)
{
    memcpy(uvars.MQTT_server, &server, server.length());
}

void Configurator::setMQTTClientName(String clientName)
{
    memcpy(uvars.MQTT_client_name, &clientName, clientName.length());
}

void Configurator::setDistanceTopic(String topic)
{
    memcpy(uvars.distance_topic, &topic, topic.length());
}

void Configurator::setLWTtopic(String LWTtopic)
{
    memcpy(uvars.lwt_topic, &LWTtopic, LWTtopic.length());
}

void Configurator::setLWTdisconnectedStatus(String dStatus)
{
    memcpy(uvars.lwt_status_disconnected, &dStatus, dStatus.length());
}

void Configurator::setLWTconnectedStatus(String cStatus)
{
    memcpy(uvars.lwt_status_running, &cStatus, cStatus.length());
}

void Configurator::setLEDbrightness(uint8_t brightness)
{
    uvars.LEDbrightness = brightness;
}