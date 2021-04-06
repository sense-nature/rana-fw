#include "FSWrapper.h"
#include "rana_logging.h"
#include <SPI.h>
#include <SD.h>
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


fs::FS & FSWrapper::getFS() 
{
    bool bSuccess = false;
    auto pFs = getSDFS( bSuccess );
    if (bSuccess){
        ESP_LOGI(TAG,"SD Card found - using as the storage");
        return *pFs;
    }
    ESP_LOGI(TAG,"SD Card NOT found - falling back to SPIFFS as the storage");
    return *getSPIFFS( bSuccess );
}

fs::FS * FSWrapper::getSPIFFS(bool & success)
{
    SPI.end();
    SPI.begin( SCK, MISO, MOSI, FLASH_CS_PIN);
    if(!SPIFFS.begin(false)){
        success = false;
        ESP_LOGE(TAG,"SPIFFS Mount Failed");
    } else {
        success = true;
        ESP_LOGI(TAG, "SPI Flash capacity used: %.3gMB of %.3gMB ",SPIFFS.usedBytes()/(1024.0*1024.0),SPIFFS.totalBytes()/(1024.0*1024.0)); 
        listDir(SPIFFS, "/",0);        
    }
    return &SPIFFS;

}

String cardTypeStr(sdcard_type_t t)
{
    switch(t){
        case CARD_NONE: return "NO CARD";
        case CARD_MMC: return "MMC CARD";
        case CARD_SD: return "SD CARD";
        case CARD_SDHC: return "SDHC CARD";
        case CARD_UNKNOWN: 
        default: return "CARD UNKNOWN";
    }
}

fs::FS * FSWrapper::getSDFS(bool & success){
    
    SPI.end();
    //turn off the LoRa SPI channel 
    //if not done, SD cannot be accessed (but SPIFFS can be)
    pinMode(LORA_CS, OUTPUT);
    digitalWrite(LORA_CS, HIGH);

    SPI.begin(SCK, MISO, MOSI, EXT_SD_CS);
    if (!SD.begin(EXT_SD_CS)) {
        ESP_LOGE(TAG,"SD init failed");
        success = false;
    } else {
        success = true; 
        ESP_LOGI(TAG, "Found SD card type: %s, size: %.3lf GB",cardTypeStr(SD.cardType()).c_str(), SD.cardSize()/(1024.0*1024.0*1024.0));
        ESP_LOGI(TAG, "Card capacity used: %.3lf GB of %.3lf GB ",SD.usedBytes()/(1024.0*1024.0*1024.0),SD.totalBytes()/(1024.0*1024.0*1024.0)); 
    }
    return &SD;
}


void FSWrapper::EndDSCardOps() 
{
    SPI.end();
    pinMode(EXT_SD_CS, OUTPUT);
    digitalWrite(EXT_SD_CS, HIGH);   
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


/**
 * Ties to deserialize the file from the path. 
 * Returns true if file was read successfully, 
 * returns fales in case of FS or deserialization error. 
 */
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
        ESP_LOGD(TAG,"Read %d element into the json root",json.size());
    }
    return result;

}


/**
 * valid modes for writing: FILE_APPEND, FILE_WRITE  ("a" | "w")
 * 
 **/
size_t FSWrapper::writeJsonDoc(const char * path, const JsonDocument & json, const char * mode) 
{
    ESP_LOGD(TAG,"Writing file: %s\n", path);

    File file = getFS().open(path, mode);
    if(!file){
        ESP_LOGE(TAG,"Failed to open file %s for writing ",path);
        return false;
    } else {
        auto res = serializeJson(json, file);
        if( strcmp(mode,FILE_APPEND) == 0 )
            file.write('\n');
        file.close();
        return res;
    }
}

