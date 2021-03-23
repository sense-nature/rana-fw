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
        if(i>0)
            result += ":";
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
    //ESP_LOGE(TAG, "Invalid hex character %d",c);
    return 0xFF;
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


String stripNonHexChars(const char * str)
{
    String out = "";
    for(int i=0; i<strlen(str); i++){
        if( hexValue(str[i]) != 0xFF )
            out += str[i]; 
    }
    return out;
}

DevAddrArray_t stringToDevAddrT(const char * str)
{


}


String devAddrToString(const DeviceAddress & da)
{
    constexpr size_t half = sizeof(DeviceAddress)/2;
    return binToHexString(da, half) +" "+binToHexString(da+half,half);
}


String devAddrToString(const DevAddrArray_t & da)
{
    constexpr size_t half = sizeof(da)/2;
    return String("")+binToHexString(da.data(), half) +"-"+binToHexString(da.data()+half,half);

    //return String("0x")+binToHexString(da.data(), da.size());
}

DevAddrArray_t toDevAddrArray(const DeviceAddress & da){
    DevAddrArray_t result;
    memcpy(result.data(), da, result.size());
    return result;
}



RtcDateTime rdtFromTimestamp(time_t ts)
{
	struct tm tms;
	gmtime_r(&ts, &tms);
	RtcDateTime rdt = RtcDateTime(tms.tm_year+1900, tms.tm_mon+1, tms.tm_mday,tms.tm_hour, tms.tm_min, tms.tm_sec);	
    return rdt;
}


String zeroLPad(const String & inp)
{
    if( inp.length() == 0 )
        return String("00");
    if( inp.length() == 1)
        return "0"+inp;
    return String(inp);
}

String rtcDTToString(const RtcDateTime & rts){
    return String(rts.Year())+"-"
            +zeroLPad(String(rts.Month()))+"-"
            +zeroLPad(String(rts.Day()))+" "
            +zeroLPad(String(rts.Hour()))+":"
            +zeroLPad(String(rts.Minute()))+":"
            +zeroLPad(String(rts.Second()))+" UTC";
}
