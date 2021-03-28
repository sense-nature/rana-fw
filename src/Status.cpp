#include "Status.h"
#include "rana_logging.h"
#include <ArduinoJson.h>
#include "FSWrapper.h"
#include <sstream>


using namespace Rana;

RTC_DATA_ATTR uint32_t Rana::staticBootCount = 0;
volatile  bool Rana::staticKeyPressChange = false;


Status::Status(){}

Status::~Status(){}

void Status::startUpNow(){
    startupTime = millis();
    wakeUpReson = esp_sleep_get_wakeup_cause();
    staticBootCount++;
    measurementCount = staticBootCount;
    ESP_LOGI(TAG, "WakeUp reason: %s", getWUResonStr() );

}

uint32_t Status::getBootCount() const 
{
    return staticBootCount;
}

bool Status::setMeasurementCount(uint32_t newCount)  
{
    if( measurementCount < newCount ){
        measurementCount = newCount;
        return true;
    }
    return false;
}

bool Status::enterConfigMode() 
{
    if( ! unknownProbeTemperatures.empty() )
        return true;
    if( staticKeyPressChange )
        return true;
    if( wakeUpReson == ESP_SLEEP_WAKEUP_ALL )
        return true;

    return false;    
}




const char * Status::getWUResonStr()
{
    static char sDefaultReason[30];
    sprintf(sDefaultReason, "Not Deep Sleep wakeup (%d)", wakeUpReson);
    switch(wakeUpReson)
    {
        case ESP_SLEEP_WAKEUP_EXT0 : return "DSWU:ext RTC_IO"; break;
        case ESP_SLEEP_WAKEUP_EXT1 : return "DSWU:ext RTC_CNTL"; break;
        case ESP_SLEEP_WAKEUP_TIMER : return "DSWU: timer"; break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD : return "DSWU: touch"; break;
        case ESP_SLEEP_WAKEUP_ULP : return "DSWU: ULP"; break;
        default: return sDefaultReason;
    }
}                          


String Status::rtcTimeStr() const 
{
    //RTCState { NoRTC, TIME_SET_FROM_DEFAULT_TIME, RTC_NOT_RUNNING, RTC_OK
    switch(rtc)
    {
        case RTCState::NoRTC : return "No ExtRTC"; 
        case RTCState::TIME_SET_FROM_DEFAULT_TIME : return  rtcDTToString(rtcUtcTime)+",  set from DEFAULT_TIME";
        case RTCState::RTC_NOT_RUNNING : return "ExtRTC Not Running";
        case RTCState::RTC_OK : return  rtcDTToString(rtcUtcTime)+",  ExtRTC OK";
    }
    return "Unknown ExtRTC status";
}



unsigned long Status::millisFromStart()
{
    return millis() - startupTime;
}

bool Status::SaveToSD() 
{
    //StaticJsonDocument<JSON_DOC_BUFFER_SIZE> json;
    DynamicJsonDocument json(3000);

    json["TS"] = rtcDTToString(rtcUtcTime);
    json["bootCount"] = staticBootCount;
    json["measurementCount"] = measurementCount;
    json["batteryLevel"] = batteryLevel;
    if(internalSensor!=SensorType::NoSensor){
        json["intTemp"]=intTemperature;
        json["intHum"]=intHumidity;
    }
    JsonObject definedProbes = json.createNestedObject("definedProbes");
    for(auto it = knownProbeTemperatures.begin(); it != knownProbeTemperatures.end(); it++){
        JsonObject kp = definedProbes.createNestedObject(String((uint8_t)it->first,10));
        kp["address"] = devAddrToString(it->second.first);
        kp["value"] = it->second.second;

    }
    JsonObject unknownProbes = json.createNestedObject("uknownProbes");
    for(auto it = unknownProbeTemperatures.begin(); it != unknownProbeTemperatures.end(); it++){
        unknownProbes[devAddrToString(it->first)] = it->second;
    }
    FSWrapper::writeJsonDoc("/status.log", json, FILE_APPEND);
    serializeJson(json, Serial);
    Serial.println("End of jsoned Status");
    return json.size();

}

