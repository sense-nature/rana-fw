#ifndef __LORAWANCONN_H__
#define __LORAWANCONN_H__

#include "Config.h"



namespace Rana{

class Device;


class LoRaWANConn
{
public:
    static void initLoRaWAN(Rana::Device &device);

public:

    LoRaWANConn();
    ~LoRaWANConn();
    void sendData(Rana::Device &device);

    void Loop();

};


} //namespace Rana

#endif // __LORAWANCONN_H__