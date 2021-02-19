#include <inttypes.h>
#include <string.h>
#include <WString.h>
#include <DallasTemperature.h>   
#include <array>


typedef std::array<uint8_t,8> DevAddrArray_t;


DevAddrArray_t toDevAddrArray(const DeviceAddress & da);
   
uint8_t hexValue(char c);
String binToHexString(const uint8_t *data, size_t size);
size_t hexStringToBin(const char * str, uint8_t * data, size_t size);;

String devAddrToString(const DeviceAddress & da);