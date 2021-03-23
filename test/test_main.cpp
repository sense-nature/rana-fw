#ifdef UNIT_TEST

#include <Arduino.h>
#include <unity.h>
#include "utils.h"

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_led_builtin_pin_number(void) {
    TEST_ASSERT_EQUAL(25, LED_BUILTIN);
}

void test_led_state_high(void) {
    digitalWrite(LED_BUILTIN, HIGH);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(LED_BUILTIN));
}

void test_led_state_low(void) {
    digitalWrite(LED_BUILTIN, LOW);
    TEST_ASSERT_EQUAL(LOW, digitalRead(LED_BUILTIN));
}


void test_bin1ToString(void)
{
    uint8_t testArr[] = {0xE2};
    String sResult = binToHexString(testArr, 1);
    String expected("E2");
    TEST_ASSERT_EQUAL_CHAR_ARRAY(expected.c_str(), sResult.c_str(),2);
}


void test_bin4ToString(void)
{
    uint8_t testArr[] = {0,0xC1,0xFB,3};
    String sResult = binToHexString(testArr, 4);
    String expected("00C1FB03");
    TEST_ASSERT_EQUAL_CHAR_ARRAY(expected.c_str(), sResult.c_str(),8);
}


void test_bin8ToString(void)
{
    uint8_t testArr[] = {0,0xC1,0xFB,3,  0xFF,1,4,254};
    String sResult = binToHexString(testArr, 4);
    String expected("00C1FB03FF0104FE");
    TEST_ASSERT_EQUAL_CHAR_ARRAY(expected.c_str(), sResult.c_str(),8);
}

void test_hexVelue(void)
{
    TEST_ASSERT_EQUAL(1, hexValue('1'));
    TEST_ASSERT_EQUAL(15, hexValue('F'));
    TEST_ASSERT_EQUAL(0xf, hexValue('f')); 
    TEST_ASSERT_EQUAL(0xA, hexValue('A'));
    TEST_ASSERT_EQUAL(hexValue('b'), hexValue('B'));
 

}


void test_hexStringTo_F0Bin(void)
{
    String sInput("F0");
    uint8_t expected[] = {0xF0};
    uint8_t testArr[] = {0};
    auto n = hexStringToBin(sInput.c_str(), testArr, 1);
    TEST_ASSERT_EQUAL(n, 1);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected,testArr,sizeof(testArr));
}

void test_hexStringTo_0FBin(void)
{
    String sInput("0F");
    uint8_t expected[] = {0x0F};
    uint8_t testArr[] = {0};
    auto n = hexStringToBin(sInput.c_str(), testArr, 1);
    TEST_ASSERT_EQUAL(n, 1);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected,testArr,sizeof(testArr));
}

void test_hexStringTo_10Bin(void)
{
    String sInput("10");
    uint8_t expected[] = {0x10};
    uint8_t testArr[] = {0};
    auto n = hexStringToBin(sInput.c_str(), testArr, 1);
    TEST_ASSERT_EQUAL(n, 1);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected,testArr,sizeof(testArr));
}


void test_hexStringToBin(void)
{
    String sInput("020310FFA0");
    uint8_t expected[5] = {0x02,0x03, 0x10, 0xFF, 0xA0};
    uint8_t testArr[5] = {0};
    auto n = hexStringToBin(sInput.c_str(), testArr, 5);
    TEST_ASSERT_EQUAL(n, 5);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected,testArr,5);
}


void test_strippingNonHex(void)
{
    TEST_ASSERT_EQUAL(stripNonHexChars("000FFf"), String("000FFf"));
    TEST_ASSERT_EQUAL(stripNonHexChars("12:34-5"), String("12345"));

}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!
    RUN_TEST(test_led_builtin_pin_number);
    RUN_TEST(test_bin1ToString);
    RUN_TEST(test_bin4ToString);
    RUN_TEST(test_bin8ToString);
    RUN_TEST(test_hexVelue);
    RUN_TEST(test_hexStringTo_10Bin);
    RUN_TEST(test_hexStringTo_0FBin);
    RUN_TEST(test_hexStringTo_F0Bin);
    RUN_TEST(test_hexStringToBin);
    RUN_TEST(test_strippingNonHex);
}

void loop() {
    UNITY_END(); // stop unit testing   
}


#endif