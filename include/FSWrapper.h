#ifndef __FSWRAPPER_H__
#define __FSWRAPPER_H__

#include <FS.h>
#include <ArduinoJson.h>


namespace Rana{
    


class FSWrapper
{
protected:
    static fs::FS *pFS; 
    //pin shown as the Flash_CS in Wireless Stick Lite
    static const uint8_t FLASH_CS_PIN = 16;

    //pin shown as the LoRa_CS
    static const uint8_t LORA_CS = 18;

    //pin connected on the PCB to the SD CS
    static const uint8_t EXT_SD_CS = 23;

    static fs::FS & getFS();
    static fs::FS * getSDFS(bool & success);
    static fs::FS * getSPIFFS(bool & success);
public:
static void begin();
    static bool readJsonDoc(const char * path, JsonDocument &json);
    static size_t writeJsonDoc(const char * path, const JsonDocument & json, const char * mode);

    static String readFileToString(const char * path); 



};


} // namespace Rana

#endif // __FSWRAPPER_H__