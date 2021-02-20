#ifndef __OWTEMPERATURES_H__
#define __OWTEMPERATURES_H__

#include "Config.h"
#include "Status.h"


namespace Rana{



class OWTemperatures
{
    
protected:
    static std::vector<std::pair<DevAddrArray_t,float>>  ReadBus(uint8_t pin);


public:
    OWTemperatures() = default;
    OWTemperatures(OWTemperatures &&) = default;
    OWTemperatures(const OWTemperatures &) = default;
    OWTemperatures &operator=(OWTemperatures &&) = default;
    OWTemperatures &operator=(const OWTemperatures &) = default;
    ~OWTemperatures() = default;

    static void ReadValues(uint8_t busPin, Config & conf, Status & status );



private:
    
};






} //namespace Rana

#endif // __OWTEMPERATURES_H__