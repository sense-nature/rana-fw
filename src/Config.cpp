#include <Arduino.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FSWrapper.h"
#include "utils.h"


using namespace Rana;


const char * Config::CONFIG_FILE = "/config.jso";



Config::Config():NodeName("DefaultNodeName")
{
}



bool Config::SaveConfig(){
    StaticJsonDocument<JSON_DOC_BUFFER_SIZE> json;
    //DynamicJsonDocument json(JSON_DOC_BUFFER);

    json[NodeName_name] = NodeName;
    json[DEVADDR_name] = binToHexString(DEVADDR, sizeof(DEVADDR));
    json[APPSKEY_name] = binToHexString(APPSKEY, sizeof(APPSKEY));
    json[NWKSKEY_name] = binToHexString(NWKSKEY, sizeof(NWKSKEY));
    json[SF_name] = SF;
    json[TimeBetween_name] = TimeBetween;
    JsonObject ProbesObj = json.createNestedObject(Probes_name);
    for(auto it = Probes.begin(); it != Probes.end(); it++)
        ProbesObj[String((uint8_t)it->first,10)] = binToHexString(it->second.data(), sizeof(DevAddrArray_t));

    //auto result = 
    FSWrapper::writeJsonDoc(CONFIG_FILE, json, FILE_WRITE);
    Serial.println("Serialized json:");
    serializeJson(json, Serial);
    Serial.println("End of json");
    return json.size();
}


void Config::ReadConfig(Status & status) 
{
    ESP_LOGD(TAG,"Loading config from file, free heap: %gKB",esp_get_free_heap_size()/1024.0);
    StaticJsonDocument<JSON_DOC_BUFFER_SIZE> json;        
    FSWrapper::readJsonDoc(CONFIG_FILE, json );

    if( json.size()  > 0 ){ 
        ESP_LOGD(TAG,"Loding config:");
        JsonVariant jv = json[NodeName_name];
        if(  ! jv.isNull()  )
            setNodeName(jv.as<const char *>());
        else
            ESP_LOGW(TAG,"NodeID not found in the config");

        jv = json[DEVADDR_name];
        if( !jv.isNull() )
            setDevaddr(json[DEVADDR_name].as<const char *>());
        else
            ESP_LOGW(TAG,"DEVADDR not found in the config");

        jv = json[SF_name];
        if( !jv.isNull() && jv.is<uint8_t>()){
            setSF(jv.as<uint8_t>());
        } else {
            ESP_LOGW(TAG,"%s is null or not uint8_t",SF_name);
        }

        jv = json[APPSKEY_name];
        if( !jv.isNull() )
            setAppskey(json[APPSKEY_name].as<const char *>());
        else
            ESP_LOGW(TAG,"APPSKEY not found in the config");

        jv = json[NWKSKEY_name];
        if( !jv.isNull() )
            setNwkskey(json[NWKSKEY_name].as<const char *>());
        else
            ESP_LOGW(TAG,"NWKSKEY not found in the config");

        jv = json[TimeBetween_name];
        if( !jv.isNull() && jv.is<uint32_t>()  ){
            setTimeBetween(jv.as<uint32_t>());
        } else {
            ESP_LOGW(TAG,"%s is null or not uint32_t",TimeBetween_name);
        }
        setProbes(json);
    } else {
        ESP_LOGW(TAG, "Empty config json");
    }
    ESP_LOGD(TAG,"End of loading config, free heap: %gKB",esp_get_free_heap_size()/1024.0);
}
    

void Config::setNodeName(const char * newName) 
{
    if(strlen(newName) > 0){
        NodeName =  newName;
        ESP_LOGD(TAG,"NodeName set to [%s]",NodeName.c_str());
    } else {
        ESP_LOGE(TAG,"Empty string passed - NodeName left untouched: [%s]",NodeName.c_str());
    }
}

void Config::setDevaddr(const char * strDevname) 
{
    auto conv = hexStringToBin(strDevname, DEVADDR, sizeof(DEVADDR));
    if(conv != sizeof(DEVADDR))
        ESP_LOGE(TAG,"Partial (%d/%d) DEVADDR definion in the config", conv,sizeof(DEVADDR));
    else
        ESP_LOGI(TAG,"Set DEVADDR with input [%s]",strDevname );
}


void Config::setSF(uint8_t newVal) 
{
    if( 7 <= newVal && newVal <= 12 ){
        SF = newVal;
        ESP_LOGD(TAG, "SF set to %d", SF);
    } else {
        ESP_LOGE(TAG, "Invalid SF value [%d], conifig untached",newVal);
    }   
}

void Config::setTimeBetween(uint32_t newVal /*[s]*/) 
{
    TimeBetween = newVal;
    ESP_LOGD(TAG,"TimeBetween measurments set to %ds",TimeBetween);

}

void Config::setAppskey(const char * strHexAppskey) 
{
   setBinaryFromHexStr(strHexAppskey, APPSKEY, (uint8_t)sizeof(APPSKEY), APPSKEY_name);
}

bool Config::setBinaryFromHexStr(const char * str, uint8_t * data, uint8_t size, const char * name ){
    auto len = hexStringToBin(str, data, size);
    if( len > 0 ){
        ESP_LOGD(TAG,"%s set to %s",name, str);
        return true;
    }else {
        ESP_LOGE(TAG,"Too short value [%s], %s not set",str, name);
        return false;
    }
}


void Config::setNwkskey(const char * strHexNwkskey) 
{
   setBinaryFromHexStr(strHexNwkskey, NWKSKEY, (uint8_t)sizeof(NWKSKEY), NWKSKEY_name);
    
}


void Config::setProbes(JsonDocument & root) 
{
    JsonObject probes = root[Probes_name];
    if( ! probes.isNull() ){
        for(uint8_t i = 0; i<0xFF; i++){
            String strI(i,10);
            const char * strAddr = probes[strI.c_str()];
            if( strAddr != nullptr  ){
                setProbeAddess(i,strAddr);
                ESP_LOGD(TAG, "Set Probes[%d]=%s",i,strAddr);
            }
        }
    } else {
        ESP_LOGE(TAG, "No or empty Probes object in the json");
   }

}


void Config::setProbeAddess(uint8_t idx, const char * strHexAddress) 
{
    DevAddrArray_t tmp;
    auto read = setBinaryFromHexStr(strHexAddress, tmp.data(), tmp.max_size(), Probes_name);
    if( read ){
        Probes[idx] = tmp;
        ESP_LOGD(TAG, "Probe[%d] set to 0x%s",idx,strHexAddress );
    } else {
        ESP_LOGE(TAG, "Invalid value [%s] for ProbeDevice address, not set",strHexAddress);
    }
}



void Config::ShowConfig() 
{
    Serial.flush();
    Serial.println("Config data: {");
    Serial.printf("  %s : %s",NodeName_name,NodeName.c_str());
    Serial.println();

    Serial.printf("  %s: 0x",DEVADDR_name);
    Serial.println(binToHexString(DEVADDR,sizeof(DEVADDR)));
    Serial.printf("  %s: 0x",APPSKEY_name);
    Serial.println(binToHexString(APPSKEY,sizeof(APPSKEY)));
    Serial.printf("  %s: 0x",NWKSKEY_name);
    Serial.println(binToHexString(NWKSKEY,sizeof(NWKSKEY)));
    
    Serial.printf("  %s:%d\n",SF_name,SF);
//    Serial.println();

    Serial.printf("  %s:%d",TimeBetween_name,TimeBetween);
    Serial.println();    

    Serial.println("  Probes: { ");

    for( auto it = Probes.begin(); it!= Probes.end(); it++){
        Serial.printf("    %d : 0x%s\n",it->first, binToHexString(it->second.data(), it->second.size() ).c_str() );
    }
    
    Serial.println("  } ");    
    Serial.println("}");
}
