#ifndef UNIT_TEST

#include <Arduino.h>
#include "rana_logging.h"
#include "RanaDevice.h"
#include "Config.h"
#include "OWTemperatures.h"


Rana::Device theDevice;

void setup() 
{
  ESP_LOGD(TAG,"theDevice pointer: 0x%X",&theDevice);
  theDevice.StartDevice();
 
 // Rana::OWTemperatures::ReadValues(config);
  ESP_LOGD(TAG,"End of the setup: free heap: %gKB",esp_get_free_heap_size()/1024.0);
}

void loop() {

  theDevice.Loop();
}


#endif