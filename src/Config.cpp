#include <Arduino.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FSWrapper.h"
#include "utils.h"


using namespace Rana;


const char * Config::CONFIG_FILE = "/config.json";



Config::Config():NodeName("DefaultNodeName")
{
}



bool Config::SaveConfig(){
    StaticJsonDocument<JSON_DOC_BUFFER_SIZE> json;
    //DynamicJsonDocument json(JSON_DOC_BUFFER);

    json[NodeName_name] = NodeName;
    json[DEVADDR_name] = binToHexString(DEVADDR, sizeof(DEVADDR));
    json[SF_name] = SF;
    auto result = FSWrapper::writeJsonDoc(CONFIG_FILE, json);
    return result == json.size(); //result < size;

}


void Config::ReadConfig() 
{
    ESP_LOGD(TAG,"Loading config from file, free heap: %gKB",esp_get_free_heap_size()/1024.0);
    StaticJsonDocument<JSON_DOC_BUFFER_SIZE> json;        
    FSWrapper::readJsonDoc(CONFIG_FILE, json);

    if( json.size()  > 0 ){ 
        ESP_LOGD(TAG,"Loding config:");

        if(json.containsKey(NodeName_name)){
            if(strlen(json[NodeName_name])>0)
                setNodeIdName((const char *) (json[NodeName_name]));
        } else {
            ESP_LOGD(TAG,"NodeID not found in the config");
        }
        if(json.containsKey(DEVADDR_name)){
           setDevaddr((const char *) (json[DEVADDR_name]));
        }else{
            ESP_LOGD(TAG,"DEVADDR not found in the config");
        }
    } else {
        ESP_LOGW(TAG, "Empty config json");
    }
    ESP_LOGD(TAG,"End of loading config, free heap: %gKB",esp_get_free_heap_size()/1024.0);
}
    

void Config::setNodeIdName(const char * newName) 
{
    NodeName =  newName;
    ESP_LOGD(TAG,"Set nodeId to [%s] by input [%s]",NodeName.c_str(), newName);
}

void Config::setDevaddr(const char * strDevname) 
{
    auto conv = hexStringToBin(strDevname, DEVADDR, sizeof(DEVADDR));
    if(conv != sizeof(DEVADDR))
        ESP_LOGE(TAG,"Partial (%d/%d) DEVADDR definion in the config", conv,sizeof(DEVADDR));
    else
        ESP_LOGI(TAG,"Set DEVADDR with input [%s]",strDevname );
}


void Config::ShowConfig() 
{
    Serial.flush();
    Serial.println("Config data: {");
    Serial.printf("  %s:%s",NodeName_name,NodeName.c_str());
    Serial.println();

    Serial.printf("  %s: 0x",DEVADDR_name);
    Serial.println(binToHexString(DEVADDR,sizeof(DEVADDR)));
    
    Serial.printf("  %s:%d",SF_name,SF);
    Serial.println();

    Serial.printf("  %s:%d",TimeBetween_name,TimeBetween);
    Serial.println();    

    Serial.println("}");
}

