#ifndef __STATUS_H__
#define __STATUS_H__


#include <inttypes.h>
#include <SSD1306.h>
#include <DallasTemperature.h>
#include <map>
#include <vector>
#include "utils.h"

#include <RtcDS3231.h>

#include <esp_wifi.h>
#include <esp_bt.h>  
#include <esp_sleep.h>



namespace Rana
{


extern RTC_DATA_ATTR uint32_t staticBootCount;
extern volatile bool staticKeyPressChange;



class Status
{
public:
    constexpr static float NotDefined = -127.0;
    enum class SensorType { NoSensor, BME280, HTU21 };
    enum class RTCState { NoRTC, SetFromBuildTime, SetFromOtherSource, RTC_OK };
    
    String nodeName; 
    esp_sleep_wakeup_cause_t wakeUpReson = ESP_SLEEP_WAKEUP_UNDEFINED;

    unsigned long startupTime = 0;   
    uint32_t measurementCount = 0;

    bool lcd = false;

    RTCState rtc = RTCState::NoRTC;
    RtcDateTime utcRtcStartupTime;

    bool sd = false;
    uint16_t batteryLevel = 0 ;
    SensorType internalSensor = SensorType::NoSensor; 
    float intTemperature = NotDefined;
    float intHumidity = NotDefined;
    float intPressure = NotDefined;
    std::map<uint8_t,std::pair<DevAddrArray_t, float>> knownProbeTemperatures; 
    std::vector<std::pair<DevAddrArray_t, float>> unknownProbeTemperatures;

    Status();
    ~Status();

    void startUpNow();
    uint32_t getBootCount() const;
    bool setMeasurementCount(uint32_t newCount);
    const char * getWUResonStr();
    unsigned long millisFromStart();

    bool enterConfigMode();




};




} //namespace Rana

#endif // __STATUS_H__