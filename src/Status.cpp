#include "Status.h"
#include "rana_logging.h"


using namespace Rana;

RTC_DATA_ATTR uint32_t Rana::staticBootCount = 0;


Status::Status(){}

Status::~Status(){}

void Status::startUpNow(){
    startupTime = millis();
    wakeUpReson = esp_sleep_get_wakeup_cause();
    staticBootCount++;
    ESP_LOGI(TAG, "WakeUp reason: %s", getWUResonStr() );

}

uint32_t Status::getBootCount() const 
{
    return staticBootCount;
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

