#include "RanaDevice.h"
#include <Arduino.h>
#include <driver/adc.h>

#include <esp_system.h>

#include <DallasTemperature.h>
#include <OneWire.h>    

#include "utils.h"

using namespace Rana;

void Device::VextON(void)
{
    pinMode(Vext,OUTPUT);
    digitalWrite(Vext, LOW);
    delay(10u);
}


void Device::VextOFF(void) 
{
    pinMode(Vext,OUTPUT);
    digitalWrite(Vext, HIGH);
}


void Device::LedON()
{
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);    
}


void Device::LedOFF()
{
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);    
}

void Device::InitSerial() 
{
    Serial.begin(115200ul);
}

/**
 * Returns a raw value from the integrated voltage divider. 
 * Max voltage ~2200, min ~1730 
 **/
uint16_t Device::RawBatteryVoltage() 
{
    //works only on the older moduled, chipid xxxx 5B1DA0D8 , not the v2.1

    //adc_power_on();
    //posssibly adc should be here initialized            
	delay(100u);
    pinMode(Battery_Pin,OPEN_DRAIN);
    delay(100u);
	auto batteryVoltage = analogRead(Battery_Pin);
    //batteryVoltage = analogRead(Battery_Pin); 
    //Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
	//Serial.println("Battery voltage reading: "+ String(batteryVoltage));
	pinMode(Battery_Pin, OUTPUT);
	digitalWrite(Battery_Pin, LOW);
    return batteryVoltage;            
}



SSD1306Wire * Device::GetDisplay() {
	static SSD1306Wire * pDisplay = 0;
	if( pDisplay == 0 ){
		pDisplay = new SSD1306Wire(0x3c, SDA_Pin, SCL_Pin, GEOMETRY_128_64);
		pDisplay->init();
		pDisplay->setFont(ArialMT_Plain_10);  
	}
	return pDisplay;
}



// function to print a device address
void printOneWireAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
	  if(i>0)
		  Serial.print(", 0x");
	  else
		  Serial.print("{ 0x");
	  if (deviceAddress[i] < 16)
		  Serial.print("0");
	  Serial.print(deviceAddress[i], HEX);
  }
  Serial.print(" }");
}  






std::vector<std::pair<DevAddrArray_t,float>>  Device::ReadDS18B20Temperatures() 
{
	std::vector<std::pair<DevAddrArray_t,float>>  temps;
    OneWire w1(OneWire_Pin);
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
        Serial.println("Detected "+String(n)+" DS18B20 sensor(s) on the bus");
		    Serial.println("Detected DS18B20 sensors:");
		    Serial.print("{");
		    for(uint8_t i=0; i < n ; i++){
				ds18b20.getAddress(addr, i);
				if(i >0)
					Serial.println(",");
				printOneWireAddress(addr);
                auto t = ds18b20.getTempC(addr);
				temps.push_back({toDevAddrArray(addr) ,t});
				Serial.print(": " + String(t)  + " *C ");
				GetDisplay()->drawString(35, i*10, String(i+1) + ": "+ String(t));
				GetDisplay()->display();
		    }
		    Serial.println("};");

		}
    pinMode(OneWire_Pin ,OUTPUT);
    digitalWrite(OneWire_Pin, LOW);     
    return temps;
}




void Device::GotoDeepSleep() 
{
	Serial.println("Going to deep sleep.");
  //from https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/issues/6#issuecomment-518896314
   pinMode(RST_LoRa,INPUT);  

   VextOFF();
   esp_deep_sleep_start();    
}

