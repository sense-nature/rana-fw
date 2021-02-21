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
  
/*

  byte error, address;
  int nDevices;

  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

*/
  delay(300);
  Rana::Device::LedOFF();
  theDevice.GotoDeepSleep();
}


#endif