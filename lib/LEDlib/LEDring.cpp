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

#include <Adafruit_NeoPixel.h>

#include <LEDring.hpp>


int LEDring::off()
{
  //Update state
  state = Off;

  //Clear all LEDs
  this->clear();

  //Send data out
  this->show();

  return (0);
}


int LEDring::setGreen()
{
  //Reset i if transitioning to green
  if(state != Green) {
    //Reset i
    this->i = 0;
  }

  //Set state to Green
  state = Green;

  return (1);
}


int LEDring::setYellow()
{
  //Reset i if transitioning to yellow
  if(state != Yellow) {
    //Reset i
    this->i = 0;
  }

  //Set state to Yellow
  state = Yellow;

  return (2);
}

/*
  val from 0 - 8
*/
int LEDring::setProgRed(int val)
{
  //Reset i if transitioning
  if(state != ProgRed) {
    this->i = 0;
  }

  //Set state to Progressive Red
  state = ProgRed;

  //Set value
  this->val = val;

  return(4);
}


int LEDring::setRed()
{
  //Reset i if transitioning to red
  if(state != Red) {
    //Reset i
    this->i = 0;
  }

  //Set state to Red
  state = Red;

  return (3);
}


int LEDring::update()
{
  //Call the appropriate state service
  switch (state)
  {
  case Green:
    GreenLightservice();
    break;
  case Yellow:
    YellowLightservice();
    break;
  case Red:
    RedLightservice();
    break;
  case ProgRed:
    ProgRedservice();
    break;
  default:
    off();
    break;
  }

  return 0;
}


int LEDring::GreenLightservice()
{
  //Side A, loop from 7 -> 0
  //Side B, loop from 8 -> 15
  //Turn all pixels off
  this->clear();
  //Set selected pixel to color
  this->setPixelColor(this->i, this->Color(0, 50, 0));
  this->setPixelColor((15 - this->i), this->Color(0, 50, 0));

  this->show();

  //Update index
  if (--this->i < 0) this->i = 7;

  return (1);
}


int LEDring::YellowLightservice()
{
  //Four pixel "chase"
  this->clear();

  this->setPixelColor(3 - this->i, this->Color(50, 50, 0));
  this->setPixelColor(7 - this->i, this->Color(50, 50, 0));
  this->setPixelColor(11 - this->i, this->Color(50, 50, 0));
  this->setPixelColor(15 - this->i, this->Color(50, 50, 0));

  this->show();

  //Update index
  if (--this->i < 0) this->i = 3;

  return (2);
}


int LEDring::ProgRedservice()
{
  this->clear();

  switch (this->val)
  {
  case 8:
    this->setPixelColor(0, this->Color(50, 0, 0));
    this->setPixelColor(15, this->Color(50, 0, 0));
  case 7:
    this->setPixelColor(1, this->Color(50, 0, 0));
    this->setPixelColor(14, this->Color(50, 0, 0));
  case 6:
    this->setPixelColor(2, this->Color(50, 0, 0));
    this->setPixelColor(13, this->Color(50, 0, 0));
  case 5:
    this->setPixelColor(3, this->Color(50, 0, 0));
    this->setPixelColor(12, this->Color(50, 0, 0));
  case 4:
    this->setPixelColor(4, this->Color(50, 0, 0));
    this->setPixelColor(11, this->Color(50, 0, 0));
  case 3:
    this->setPixelColor(5, this->Color(50, 0, 0));
    this->setPixelColor(10, this->Color(50, 0, 0));
  case 2:
    this->setPixelColor(6, this->Color(50, 0, 0));
    this->setPixelColor(9, this->Color(50, 0, 0));
  case 1:
    this->setPixelColor(7, this->Color(50, 0, 0));
    this->setPixelColor(8, this->Color(50, 0, 0));
    break;
  default:
    break;
  }

  this->show();

  return(4);
}


int LEDring::RedLightservice()
{
  static int intensity = 255;
  this->clear();

  for (int j = 0; j < 16; j++) {
    this->setPixelColor(j, this->Color(intensity, 0 , 0));
  }

  this->show();

  //Update intensity
  intensity -= 256 / 8;
  if (intensity < 0) intensity = 255;

  return (3);
}