#ifndef __Rana_Device_h__
#define __Rana_Device_h__

#include <inttypes.h>
#include <SSD1306.h>
#include <DallasTemperature.h>
#include <map>
#include <vector>
#include "utils.h"

#include <esp_wifi.h>
#include <esp_bt.h>  
#include <esp_sleep.h>

#include "Config.h"
#include "Status.h"
#include "LoRaWANConn.h"


namespace Rana
{
    

class Device 
{
protected:
    const static uint8_t OneWire_Pin = 4;
    const static uint8_t SDA_Pin = 15;
    const static uint8_t SCL_Pin = 2;

    const static uint8_t Battery_Pin = 13;



    

    LoRaWANConn lora;

public:
    Status status;
    Config config;

    void StartDevice();
    void InitLCD();

    void VextON();
    void VextOFF();

    static void LedON();
    static void LedOFF();


    void UpdateRTCData();
    void UpdateEepromData();
    void GetInternalSensorValues();
    void GetBatteryLevel();

	void startWebConfig();
    void sendMeasurementsOveLoRaWAN();

    static SSD1306Wire * GetDisplay();
    static std::vector<std::pair<DevAddrArray_t,float>> ReadDS18B20Temperatures();

    void ReadConfigFromSD();
    void SaveConfigToSD();

    void Loop();
    void GotoDeepSleep();
};









} // namespace Rana


#endif