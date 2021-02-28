#ifndef __LORAWANCONN_H__
#define __LORAWANCONN_H__

#include "Config.h"



namespace Rana{

class Device;


class LoRaWANConn
{
public:

    LoRaWANConn();
    ~LoRaWANConn();
    static void initLoRaWAN(uint32_t seqNo, Config &conf);
    void sendData(Rana::Device &device);

    void Loop();

};


} //namespace Rana

#endif // __LORAWANCONN_H__