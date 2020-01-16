#if defined(ARDUINO) && defined(UNIT_TEST)

#include <Arduino.h>
#include "unity.h"

#include <Adafruit_NeoPixel.h>
#include <LEDring.hpp>

void test_LEDs()
{
    //LED funtion ptr
    LEDring pixels(16, 5, NEO_GRB + NEO_KHZ800);

    TEST_ASSERT_EQUAL(pixels.off(), 0);
}

// setup connects serial, runs test cases
void setup() {
    delay(2000);

    //
    UNITY_BEGIN();

    //calls to test go here
    RUN_TEST(test_LEDs);

    //
    UNITY_END();
}

void loop() {
    //nothing to be done here
}

#endif