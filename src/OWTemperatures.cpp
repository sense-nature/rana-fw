#include "OWTemperatures.h"
#include "RanaDevice.h"
#include "utils.h"
#include <DallasTemperature.h>
#include <set>

using namespace Rana;



bool areEqual(const DevAddrArray_t & lhs, const DevAddrArray_t & rhs)
{
    for(size_t i = 0; i<sizeof(DeviceAddress); i++)
        if( lhs[i] != rhs[i] )
            return false;
    return true;
}

void OWTemperatures::ReadValues(uint8_t pin, Config & conf, Status &status) 
{
    status.knownProbeTemperatures.clear();
    status.unknownProbeTemperatures.clear();
    auto temps = ReadBus(pin);
    std::set<uint8_t> unknown;
    for(size_t i =0; i<temps.size(); i++)
        unknown.insert((uint8_t)i);
    for( auto & x : conf.Probes){
        bool found = false;
        size_t tIndex = 0;
        for(size_t i = 0 ; i < temps.size(); i++){
//            auto & addr = temps[i].first;
            if( areEqual(temps[i].first, x.second) ){
                found = true;
                tIndex = i;
                break;
            }
        }
        if(found){
            status.knownProbeTemperatures[x.first]={x.second,  temps[tIndex].second};
            Serial.printf("T%d [%s] = %.2f *C\n", x.first, binToHexString(x.second.data(),x.second.size()).c_str(), temps[tIndex].second);
            unknown.erase(tIndex);
        } else {
            Serial.printf("T%d [%s] NOT_FOUND\n", x.first, binToHexString(x.second.data(),x.second.size()).c_str());
        }
    }
    if( ! unknown.empty() ){
        Serial.println("Unknown probes: ");
        for( auto i : unknown){
            Serial.printf("[0x%s] = %.2f *C \n", binToHexString(temps[i].first.data(), temps[i].first.size()).c_str(),temps[i].second);
            status.unknownProbeTemperatures.push_back({temps[i].first, temps[i].second});
        }
    }
}



std::vector<std::pair<DevAddrArray_t,float>>  OWTemperatures::ReadBus(uint8_t pin) 
{
	std::vector<std::pair<DevAddrArray_t,float>>  temps;
    OneWire w1(pin);
	DallasTemperature ds18b20(&w1);
	ds18b20.begin();
	ds18b20.setResolution(12);
	ds18b20.setCheckForConversion(false);
	ds18b20.requestTemperatures();
	delay(200u);
	uint8_t n = ds18b20.getDS18Count();
	temps.reserve(n);
	 if( n > 0 ){
        DeviceAddress addr = {0};
        ESP_LOGD(TAG, "Detected %d DS18B20 sensor(s) on the bus",n);
        for(uint8_t i=0; i < n ; i++){
            ds18b20.getAddress(addr, i);
            auto t = ds18b20.getTempC(addr);
            temps.push_back({toDevAddrArray(addr) ,t});
        }
    }
    pinMode(pin ,OUTPUT);
    digitalWrite(pin, LOW);     
    return temps;
}

