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

#if defined(ARDUINO) && defined(UNIT_TEST)

#include <Arduino.h>
#include "unity.h"

#include <Adafruit_NeoPixel.h>
#include <LEDring.hpp>

    LEDring pixels(16, 5, NEO_GRB + NEO_KHZ800);

void test_LED_Green()
{
    TEST_ASSERT_EQUAL(pixels.setGreen(), 1);
    delay(1000);
}

void test_LED_Yellow()
{
    TEST_ASSERT_EQUAL(pixels.setYellow(), 2);
    delay(1000);
}

void test_LED_Red()
{
    TEST_ASSERT_EQUAL(pixels.setRed(), 3);
    delay(1000);
}

void test_LED_off()
{
    TEST_ASSERT_EQUAL(pixels.off(), 0);
}

// setup connects serial, runs test cases
void setup() {
    delay(2000);

    //
    UNITY_BEGIN();

    //calls to test go here
    RUN_TEST(test_LED_Green);

    RUN_TEST(test_LED_Yellow);

    RUN_TEST(test_LED_Red);

    RUN_TEST(test_LED_off);

    //
    UNITY_END();
}

void loop() {
    //nothing to be done here
}

#endif