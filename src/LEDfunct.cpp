#include <Adafruit_NeoPixel.h>

static int i = 0;

void GreenLight(Adafruit_NeoPixel pixel)
{
  //Side A, loop from 7 -> 0
  //Side B, loop from 8 -> 15
  //Turn all pixels off
  pixel.clear();
  //Set selected pixel to color
  pixel.setPixelColor(i, pixel.Color(0, 50, 0));
  pixel.setPixelColor((15 - i), pixel.Color(0, 50, 0));

  pixel.show();

  //Update index
  if (--i < 0) i = 7;
}


void YellowLight(Adafruit_NeoPixel pixel)
{
  //Four pixel "chase"
  pixel.clear();

  pixel.setPixelColor(3 - i, pixel.Color(50, 50, 0));
  pixel.setPixelColor(7 - i, pixel.Color(50, 50, 0));
  pixel.setPixelColor(11 - i, pixel.Color(50, 50, 0));
  pixel.setPixelColor(15 - i, pixel.Color(50, 50, 0));

  pixel.show();

  //Update index
  if (--i < 0) i = 3;
}


void RedLight(Adafruit_NeoPixel pixel)
{
  static int intensity = 255;
  pixel.clear();

  for (int j = 0; j < 16; j++) {
    pixel.setPixelColor(j, pixel.Color(intensity, 0 , 0));
  }

  pixel.show();

  //Update intensity
  intensity -= 256 / 8;
  if (intensity < 0) intensity = 255;
}


void LEDoff(Adafruit_NeoPixel pixel)
{
  pixel.clear();

  pixel.show();
}