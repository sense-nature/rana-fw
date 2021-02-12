#ifndef __FSWRAPPER_H__
#define __FSWRAPPER_H__

#include <FS.h>
#include <ArduinoJson.h>


namespace Rana{
    


class FSWrapper
{
protected:
    static fs::FS *pFS; 
    //pin for shown as the Flash_CS in Wireless Stick Lite
    static const uint8_t FLASH_CS_PIN = 16;

    static fs::FS & getFS();

public:
static void begin();
    static bool readJsonDoc(const char * path, JsonDocument &json);
    static size_t writeJsonDoc(const char * path, const JsonDocument & json);

    static String readFileToString(const char * path); 
    static size_t appendToFile(const char *path, const String & data);


};


} // namespace Rana

#endif // __FSWRAPPER_H__