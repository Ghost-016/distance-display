
#include <Adafruit_NeoPixel.h>

#include <LEDring.hpp>


int LEDring::GreenLight()
{
#ifndef UNIT_TEST
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
#endif
  return (1);
}


int LEDring::YellowLight()
{
#ifndef UNIT_TEST
  //Four pixel "chase"
  this->clear();

  this->setPixelColor(3 - this->i, this->Color(50, 50, 0));
  this->setPixelColor(7 - this->i, this->Color(50, 50, 0));
  this->setPixelColor(11 - this->i, this->Color(50, 50, 0));
  this->setPixelColor(15 - this->i, this->Color(50, 50, 0));

  this->show();

  //Update index
  if (--this->i < 0) this->i = 3;
#endif
  return (2);
}


int LEDring::RedLight()
{
#ifndef UNIT_TEST
  static int intensity = 255;
  this->clear();

  for (int j = 0; j < 16; j++) {
    this->setPixelColor(j, this->Color(intensity, 0 , 0));
  }

  this->show();

  //Update intensity
  intensity -= 256 / 8;
  if (intensity < 0) intensity = 255;
#endif
  return (3);
}


int LEDring::off()
{
#ifndef UNIT_TEST
  this->clear();

  this->show();
#endif
  return (0);
}