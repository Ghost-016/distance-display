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


#ifndef CONFIGSTATEMACHINE_HPP
#define CONFIGSTATEMACHINE_HPP

#include <StateMachine.h>

class ConfigData : public EventData
{
public:
    char input;
};

class ConfigMachine : public StateMachine
{
public:
    ConfigMachine();

    //External events taken by this state machine
    void MainReturn(ConfigData* data);
    void DistanceMenu(ConfigData* data);
    void UploadMenu(ConfigData* data);
    void MQTTMenu(ConfigData* data);
    void LEDMenu(ConfigData* data);

private:
    //State enumeration order must match the order of state method entries in the state map
    enum States
    {
        ST_MAIN_MENU,
        ST_DISTANCE,
        ST_UPLOAD,
        ST_MQTT,
        ST_LED,
        ST_USER_INPUT,
        ST_MAX_STATES
    };

    //Define the state machine state functions with event data type
    STATE_DECLARE(ConfigMachine,    MainMenu,          ConfigData)
    STATE_DECLARE(ConfigMachine,    Distance,          ConfigData)
    STATE_DECLARE(ConfigMachine,    Upload,            ConfigData)
    STATE_DECLARE(ConfigMachine,    Mqtt,              ConfigData)
    STATE_DECLARE(ConfigMachine,    Led,               ConfigData)
    STATE_DECLARE(ConfigMachine,    User_Input,        ConfigData)

    //State map to define state object order. Each state map entry defines a state object
    BEGIN_STATE_MAP 
        STATE_MAP_ENTRY(&MainMenu)
        STATE_MAP_ENTRY(&Distance)
        STATE_MAP_ENTRY(&Upload)
        STATE_MAP_ENTRY(&Mqtt)
        STATE_MAP_ENTRY(&Led)
        STATE_MAP_ENTRY(&User_Input)
    END_STATE_MAP
};

#endif //CONFIGSTATEMACHINE_HPP