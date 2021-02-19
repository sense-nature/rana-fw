#ifndef __OWTEMPERATURES_H__
#define __OWTEMPERATURES_H__

#include "Config.h"


namespace Rana{



class OWTemperatures
{
    

public:
    OWTemperatures() = default;
    OWTemperatures(OWTemperatures &&) = default;
    OWTemperatures(const OWTemperatures &) = default;
    OWTemperatures &operator=(OWTemperatures &&) = default;
    OWTemperatures &operator=(const OWTemperatures &) = default;
    ~OWTemperatures() = default;

    static void ReadValues( Config & conf );


private:
    
};






} //namespace Rana

#endif // __OWTEMPERATURES_H__