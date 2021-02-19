#include "../../include/rana_logging.h"
#include "utils.h"
#include <stdio.h>


String binToHexString(const uint8_t *data, size_t size)
{
    //const uint8_t zero = '0';
    String result, temp("  ");   
    result.reserve(size * 2);
    for(int i = 0 ; i < size; i++){
        sprintf(temp.begin(), "%.2X",data[i]);
        result += temp;
    }
    return result;
}


uint8_t hexValue(char c){
    const uint8_t zero = '0';
    const uint8_t nine = '9';
    const uint8_t A = 'A';
    const uint8_t F = 'F';
    const uint8_t a = 'a';
    const uint8_t f = 'f';
    if( zero <= c && c<= nine )
        return c - zero;
    if( A <= c && c <= F )
        return 10 + (c - A );
    if( a <= c && c <= f )
        return 10 + (c - a );
    ESP_LOGE(TAG, "Invalid hex character %d",c);
    return 0;
}

size_t hexStringToBin(const char * str, uint8_t * data, size_t size)
{
    size_t result = 0;
    auto len = strlen(str);
    if( len <  2 * size)
        return 0;
    memset(data,0,size);
    for(int i = 0 ; i < size; i++){
        data[i] = hexValue(str[2*i]) * 16 +  hexValue(str[2*i+1]) ;
        result ++;
    }
    return result;
}


String devAddrToString(const DeviceAddress & da)
{
    return String("0x")+binToHexString(da, sizeof(DeviceAddress));
}


DevAddrArray_t toDevAddrArray(const DeviceAddress & da){
    DevAddrArray_t result;
    memcpy(result.data(), da, result.size());
    return result;
}
