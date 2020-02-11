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
    Initializes LED ring

    Loop at 10hz, change pattern based on distance

    Loop at 1hz, check timeout of distance to turn LEDs off

    Inputs: distance
            LEDtimeout
            uvars
    Outputs: LEDs via LEDring class

    Hardware requriements: neopixel capable pin
*/

#include "display.hpp"

const int NEOPIXEL_PIN = D1;
const int NUMPIXELS = 16;

/*

*/
void display::begin(const Configurator &config, NTPClient &timeClient)
{
  //assign the config class pointer for internal use
  m_config = &config;

  //assign the NTP time client pointer for internal use
  m_timeClient = &timeClient;

  //Initialize LED ring
  ring = new LEDring(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

  //NeoPixel init
  ring->begin();
}


/*
    
*/
void display::loop(float distance)
{
  if(!isnan(distance) && (m_LEDenabled == true)) {
    //Call LED function
    if ((distance > m_config->getFarDistance()) || (distance < 0)) {
      ring->setGreen();
    }
    else if ((distance < m_config->getMidDistance()) && (distance > (m_config->getNearDistance() + m_config->getHystDistance()))) {
      ring->setYellow();
    }
    else if ((distance < m_config->getNearDistance()) && (distance > 0.0)) {
      ring->setRed();
    }
  }
  else {
    ring->off();
  }

  //update LEDs regularly
  ring->update();
 #if 1
  //Check if distance has changed by hystDistance
  if (checkBound(distance, m_prevdistance, m_config->getHystDistance())) {
    //update previous distance
    m_prevdistance = distance;
    //update lastEpochTime to reset the timeout
    m_lastEpochTime = m_timeClient->getEpochTime();
    //enable LEDs
    m_LEDenabled = true;
  }
  //if its been a significant ammount of time since a change
  else if(m_timeClient->getEpochTime() - m_lastEpochTime >= m_config->getLEDTimeout()) {
    m_lastEpochTime = m_timeClient->getEpochTime();
    //disable LEDs
    m_LEDenabled = false;
  }
  #endif
}
    



//Returns true if newVal is greater or less than prevVal by maxDiff
bool display::checkBound(float newVal, float prevVal, float maxDiff) {
  return (!isnan(newVal) && ((newVal < prevVal - maxDiff) || (newVal > prevVal + maxDiff)));
}