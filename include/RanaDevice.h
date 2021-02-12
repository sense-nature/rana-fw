#ifndef __Rana_Device_h__
#define __Rana_Device_h__

#include <inttypes.h>
#include <SSD1306.h>

#include <esp_wifi.h>
#include <esp_bt.h>  



namespace Rana
{
    


class Device 
{
protected:
    const static uint8_t OneWire_Pin = 4;
    const static uint8_t SDA_Pin = 15;
    const static uint8_t SCL_Pin = 2;

    const static uint8_t Battery_Pin = 13;

public:
    static void InitSerial();
    static void InitLCD();

    static void VextON();
    static void VextOFF();

    static void LedON();
    static void LedOFF();


    static uint16_t RawBatteryVoltage();
    static SSD1306Wire * GetDisplay();
    static std::vector<float> ReadDS18B20Temperatures();

    void ReadConfigFromSD();
    void SaveConfigToSD();

         
};









} // namespace Rana


#endif