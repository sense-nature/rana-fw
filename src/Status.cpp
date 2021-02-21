#include "Status.h"
#include "rana_logging.h"


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
    sprintf(sDefaultReason, "NotDS: %d", wakeUpReson);
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

