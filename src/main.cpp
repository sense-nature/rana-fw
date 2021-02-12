#ifndef UNIT_TEST

#include <Arduino.h>
#include "rana_logging.h"
#include "RanaDevice.h"
#include "Config.h"

Rana::Config config;


void setup() 
{
  Rana::Device::InitSerial();
  ESP_LOGI(TAG, "Rana start");
  ESP_LOGD(TAG,"At setup start : free heap: %gKB",esp_get_free_heap_size()/1024.0);
  Serial.printf("Heap size %gkB", esp_get_free_heap_size()/1024.0);
  esp_bt_controller_disable();       
  Rana::Device::VextON();
  Rana::Device::GetDisplay();
  config.ReadConfig();
  config.ShowConfig();
  config.SaveConfig();

  ESP_LOGD(TAG,"At setup end: free heap: %gKB",esp_get_free_heap_size()/1024.0);
}

void loop() {

  Rana::Device::LedON();
  delay(1000);
  Rana::Device::LedOFF();
  Serial.println("Led OFF");
  delay(8000);
}


#endif