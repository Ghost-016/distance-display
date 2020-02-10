#if defined(ARDUINO) && defined(UNIT_TEST)

#include <Arduino.h>
#include "unity.h"

#include "configurator.hpp"

Configurator config;

void test_distances()
{
    float far = config.getFarDistance();
    TEST_ASSERT_EQUAL(far , 200.0);

    float mid = config.getMidDistance();
    TEST_ASSERT_EQUAL(mid, 75.0);

    float near = config.getNearDistacne();
    TEST_ASSERT_EQUAL(near, 40.0);
}

void test_upload_parameters()
{
    String uploadName = config.getUploadUsername();
    TEST_ASSERT_EQUAL_STRING(uploadName.c_str(), "admin");

    String uploadPassword = config.getUploadPassword();
    TEST_ASSERT_EQUAL_STRING(uploadPassword.c_str(), "admin");
}

void test_MQTT()
{
    String MQTTServer = config.getMQTTServer();
    TEST_ASSERT_EQUAL_STRING(MQTTServer.c_str(), "192.168.2.103");

    String MQTTClientName = config.getMQTTClientName();
    TEST_ASSERT_EQUAL_STRING(MQTTClientName.c_str(), "ESP8266_GARAGE");

    String MQTTDistanceTopic = config.getDistanceTopic();
    TEST_ASSERT_EQUAL_STRING(MQTTDistanceTopic.c_str(), "sensor/garage/distance");

    String MQTTLWTTopic = config.getLWTTopic();
    TEST_ASSERT_EQUAL_STRING(MQTTLWTTopic.c_str(), "sensor/garage/status");

    String MQTTLWTDisconnected = config.getLWTDisconnected();
    TEST_ASSERT_EQUAL_STRING(MQTTLWTDisconnected.c_str(), "disconnected");

    String MQTTLWTConnected = config.getLWTConnected();
    TEST_ASSERT_EQUAL_STRING(MQTTLWTConnected.c_str(), "connected");
}

void setup()
{
    delay(2000);

    //Don't have to call this as long as save is not called
    //config.begin();

    UNITY_BEGIN();

    RUN_TEST(test_distances);

    RUN_TEST(test_upload_parameters);

    RUN_TEST(test_MQTT);

    UNITY_END();
}


void loop()
{

}

#endif