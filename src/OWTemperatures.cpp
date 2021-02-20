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

void OWTemperatures::ReadValues(Config & conf, Status &status) 
{
    status.knownProbeTemperatures.clear();
    status.unknownProbeTemperatures.clear();
    auto temps = Rana::Device::ReadDS18B20Temperatures();
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