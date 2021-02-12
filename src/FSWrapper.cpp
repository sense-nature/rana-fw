#include "FSWrapper.h"
#include "rana_logging.h"
#include <SPI.h>
#include <SPIFFS.h>



using namespace Rana;


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        ESP_LOGE(TAG,"%s - failed to open directory",dirname);
        return;
    }
    if(!root.isDirectory()){
        ESP_LOGE(TAG,"%s - not a directory", dirname);
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}





fs::FS * FSWrapper::pFS = nullptr;

fs::FS & FSWrapper::getFS() 
{
    if( pFS == nullptr ){
        SPI.begin( SCK, MISO, MOSI, FLASH_CS_PIN);
        if(!SPIFFS.begin(true)){
            ESP_LOGE(TAG,"SPIFFS Mount Failed");
        } else {
            listDir(SPIFFS, "/",0);        
        }
        pFS = & SPIFFS;
        return *pFS;
    } 

    return *pFS;
}


String FSWrapper::readFileToString(const char * path) 
{
    //return readFile(getFS(),path);
    ESP_LOGD(TAG,"Reading file: %s\n", path);
    File file = getFS().open(path, FILE_READ);
    String sResult;
    if(!file){
        ESP_LOGE(TAG,"Failed to open file [%s] for reading",path);
    } else {
        sResult = file.readString();
        file.close();
        ESP_LOGD(TAG,"Read %d bytes into string",sResult.length());
    }
    return sResult;
}


bool FSWrapper::readJsonDoc(const char * path, JsonDocument & json) 
{
    ESP_LOGD(TAG,"Deserializing json file: %s\n", path);
    File file = getFS().open(path, FILE_READ);
    bool result = false;
    if(!file){
        ESP_LOGE(TAG,"Failed to open file [%s] for reading",path);
    } else {
        auto error = deserializeJson(json, file);
        if( error ){
            ESP_LOGE(TAG,"%s", error.c_str());
        } else {
            result = true;
            ESP_LOGD(TAG,"Successfully read json config");
        }
        file.close();
        ESP_LOGD(TAG,"Read %d bytes into string",json.size());
    }
    return result;

}


/**
 * valid modes for writing: FILE_APPEND, FILE_WRITE  ("a" | "w")
 * 
 **/
size_t FSWrapper::writeJsonDoc(const char * path, const JsonDocument & json) 
{
    ESP_LOGD(TAG,"Writing file: %s\n", path);

    File file = getFS().open(path, FILE_WRITE);
    if(!file){
        ESP_LOGE(TAG,"Failed to open file %s for writing ",path);
        return false;
    } else {
        auto res = serializeJson(json, file);
        file.close();
        return res;
    }
}


size_t FSWrapper::appendToFile(const char *path, const String & data) 
{

    return 0;
}

