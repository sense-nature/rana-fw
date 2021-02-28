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
        
    return false;    
}




const char * Status::getWUResonStr()
{
    static char sDefaultReason[30];
    sprintf(sDefaultReason, "NotDSWU (%d)", wakeUpReson);
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


unsigned long Status::millisFromStart()
{
    return millis() - startupTime;
}

String zeroLPad(String inp)
{
    if( inp.length() == 0 )
        return String("00");
    if( inp.length() == 1)
        return "0"+inp;
    return inp;
}

String RtcTSToString(const RtcDateTime & rts){
    return String(rts.Year())+"-"
            +zeroLPad(String(rts.Month()))+"-"
            +zeroLPad(String(rts.Day()))+" "
            +zeroLPad(String(rts.Hour()))+":"
            +zeroLPad(String(rts.Minute()))+":"
            +zeroLPad(String(rts.Second()))+"UTC";
}

bool Status::SaveToSD() 
{
    //StaticJsonDocument<JSON_DOC_BUFFER_SIZE> json;
    DynamicJsonDocument json(3000);

    json["TS"] = RtcTSToString(utcRtcStartupTime);
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

