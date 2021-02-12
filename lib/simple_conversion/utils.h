#include <inttypes.h>
#include <string.h>
#include <WString.h>


uint8_t hexValue(char c);
String binToHexString(const uint8_t *data, size_t size);
size_t hexStringToBin(const char * str, uint8_t * data, size_t size);