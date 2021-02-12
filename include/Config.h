#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <FS.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <DallasTemperature.h>   
#include "FSWrapper.h"



namespace Rana
{

class Config
{
protected:
    static const char * CONFIG_FILE;
    static const size_t JSON_DOC_BUFFER_SIZE = 2000;
    static const size_t NODE_NAME_MAX_LENGTH = 32;




    void setNodeIdName(const char * newName);
    void setDevaddr(const char * strDevname);
    void setAppskey(const char * strAppskey);
    void setNwkskey(const char * strNwkskey);


public:

    String NodeName;
    const char * NodeName_name = "NodeName";

    uint8_t DEVADDR[4] = {0};
    const char * DEVADDR_name = "DEVADDR";

    uint8_t SF = 10;
    const char * SF_name = "SF";

/**
 * Time between measurements, in seconds
 */
    uint32_t TimeBetween = 900; 
    const char * TimeBetween_name = "TimeBetween";

    uint8_t APPSKEY[16] = {0};
    const char * APPSKEY_name = "APPSKEY";

    uint8_t NWKSKEY[16] = {0};
    const char * NWKSKEY_name = "NWKSKEY";
    
    std::vector<DeviceAddress> Probes;
    const char * probes_name = "Probes";
    


    Config();
    ~Config(){};
    //fallbacking to SPIFS for the config
    //the SD still doesn't want to work
    //const static uint8_t SD_CS_Pin = 23;

    bool SaveConfig();
    void ReadConfig();
    void ShowConfig();

};

 
} // namespace Rana

#endif // __CONFIG_H__