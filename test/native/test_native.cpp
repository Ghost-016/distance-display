#ifdef UNIT_TEST

#include <unity.h>

//#include <Adafruit_NeoPixel.h>
#include <LEDring.hpp>

void test_LEDs()
{
    LEDring ring(16, 5, NEO_GRB + NEO_KHZ800);

    TEST_ASSERT_EQUAL(ring.off(), 0);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_LEDs);

    UNITY_END();
}

#endif