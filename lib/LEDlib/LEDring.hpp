#ifndef LEDRING_HPP
#define LEDRING_HPP
class LEDring: public Adafruit_NeoPixel {
public:
    LEDring(uint16_t n, uint16_t pin=6, neoPixelType type=NEO_GRB + NEO_KHZ800) 
        : Adafruit_NeoPixel(n, pin, type) {}
    int GreenLight();
    int YellowLight();
    int RedLight();
    int off();
private:
    int i;
};
#endif  //LEDRING_HPP