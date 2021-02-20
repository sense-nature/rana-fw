#ifndef UNIT_TEST

#include <Arduino.h>
#include "rana_logging.h"
#include "RanaDevice.h"
#include "Config.h"
#include "OWTemperatures.h"


Rana::Device theDevice;

void setup() 
{
  theDevice.StartDevice();
 
 
 // Rana::OWTemperatures::ReadValues(config);
  ESP_LOGD(TAG,"At setup end: free heap: %gKB",esp_get_free_heap_size()/1024.0);
}

void loop() {

  Rana::Device::LedON();
  delay(1000);
  Rana::Device::LedOFF();
  //Serial.println("Led OFF");
  delay(1000);
  theDevice.GotoDeepSleep();
}


#endif