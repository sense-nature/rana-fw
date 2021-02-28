#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <FS.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <DallasTemperature.h>   
#include "FSWrapper.h"
#include "utils.h"
#include "Status.h" 
#include <map>



namespace Rana
{



class Config
{
protected:
    static const char * CONFIG_FILE;
    static const size_t JSON_DOC_BUFFER_SIZE = 2000;
    static const size_t NODE_NAME_MAX_LENGTH = 32;

    void setNodeName(const char * newName);
    void setDevaddr(const char * strDevname);
    void setSF(uint8_t newVal);
    void setTimeBetween(uint32_t newVal);
    void setAppskey(const char * strHexAppskey);
    void setNwkskey(const char * strHexNwkskey);
    
    void setProbes(JsonDocument & root);
    void setProbeAddess(uint8_t idx, const char * strHexAddress);

    bool setBinaryFromHexStr(const char * str, uint8_t * data, uint8_t size, const char * name );
public:
    String NodeName;
    const char * NodeName_name = "NodeName";

    uint8_t DEVADDR[4] = {0};
    const char * DEVADDR_name = "DEVADDR";

    uint8_t SF = 9;
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
    
    std::map<uint8_t,DevAddrArray_t> Probes;
    const char * Probes_name = "Probes";
    


    Config();
    ~Config(){};
    bool SaveConfig();
    void ReadConfig( Status & status );
    void ShowConfig();

    uint32_t DEVADDRu32() const;


};



 
} // namespace Rana

#endif // __CONFIG_H__