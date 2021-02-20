#include "RanaDevice.h"
#include <Arduino.h>
#include <RtcDS3231.h>
#include <EepromAT24C32.h>

#include <driver/adc.h>

#include <esp_system.h>

#include <DallasTemperature.h>
#include <OneWire.h>    
#include <SPI.h>
#include "OWTemperatures.h"

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

void Device::StartDevice() 
{
    status.startUpNow();
	Serial.begin(115200ul);
	Rana::Device::VextON();
	Wire.begin(SDA_Pin, SCL_Pin);
	ReadRTCData();



	ESP_LOGI(TAG, "Rana start");
	ESP_LOGD(TAG, "At setup start : free heap: %gKB",esp_get_free_heap_size()/1024.0);	
	esp_bt_controller_disable();       
	Rana::Device::GetDisplay();
	config.ReadConfig();
	config.ShowConfig();
	OWTemperatures::ReadValues(OneWire_Pin, config ,status);

}

void Device::ReadRTCData()
{
	const RtcDateTime compilation = RtcDateTime(__DATE__, __TIME__);
    ESP_LOGI(TAG,"Compilation date: %4u-%2u-%2u %2u:%2u:%2u", 
		compilation.Year(), 
		compilation.Month(), 
		compilation.Day(), 
		compilation.Hour(), 
		compilation.Minute(),
		compilation.Second()
	);

	RtcDS3231<TwoWire> rtc(Wire);
	rtc.Begin();
	bool readEeprom = false;
	if( ! rtc.IsDateTimeValid() ){
		if( rtc.LastError() != 0 ){
			ESP_LOGE(TAG, "RTC I2C communication error %d",rtc.LastError() );
		} else { 
			ESP_LOGE(TAG, "RTC low battery or similar error");
			readEeprom = true;
		}
	} 
	if( rtc.LastError() == 0 ) {
		if( ! rtc.GetIsRunning() ){
			ESP_LOGE(TAG, "RTC not running");
		} else {
			RtcDateTime now = rtc.GetDateTime();
    		if (now <  compilation ) {
				readEeprom = false;
				rtc.SetIsRunning(false);
				ESP_LOGI(TAG,"RTC time earlier than compitaltion time -> stopping");
		    } else {
				status.rtc = true;
				status.utcRtcStartupTime = rtc.GetDateTime();
				ESP_LOGI(TAG,"Set UTC status time to RTC %4u-%2.2u-%2.2u %2u:%2u:%2u", 
					status.utcRtcStartupTime.Year(), 
					status.utcRtcStartupTime.Month(), 
					status.utcRtcStartupTime.Day(), 
					status.utcRtcStartupTime.Hour(), 
					status.utcRtcStartupTime.Minute(),
					status.utcRtcStartupTime.Second()
				);
			}
		}
		rtc.Enable32kHzPin(false);
    	rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
	}

	EepromAt24c32<TwoWire> eeprom(Wire);
	eeprom.Begin();
	const uint16_t valueAddress = 0;


	ESP_LOGD(TAG,"Current boot count: %u, measurement count: %u", staticBootCount, status.measurementCount);
	if( readEeprom && eeprom.LastError() == 0 ){
		uint32_t eepromValue = 0;
		auto res = eeprom.GetMemory(valueAddress, (uint8_t*)&eepromValue, sizeof(eepromValue));
		Wire.flush();
		ESP_LOGI(TAG,"AT24c32 EEPROM: Read %uB, value=%u  ", res, eepromValue);
		if( eeprom.LastError() != 0 ){
			ESP_LOGE(TAG,"Reading form EEPROM error=%d",eeprom.LastError());
		} else {
			if( res == sizeof(eepromValue) && eepromValue >= staticBootCount ){
				eepromValue++;
				status.setMeasurementCount(eepromValue);
				ESP_LOGI(TAG, "Updated measurementCount to %u",eepromValue);
			} else {
				ESP_LOGI(TAG, "Read %B-value smaller then the current bootCount %u",res,staticBootCount);
			}
		}
	}
	if( eeprom.LastError() == 0 ){
		eeprom.SetMemory(valueAddress, (uint8_t*)&status.measurementCount, sizeof(status.measurementCount));
		if( eeprom.LastError() == 0 )
			ESP_LOGI(TAG,"Written %u current measurement number to the EEPROM",status.measurementCount);
		else			
			ESP_LOGE(TAG,"Error %d on writng to the EEPROM", eeprom.LastError());
	} else {
		ESP_LOGE(TAG,"No writing to the EEPROM because of earlier error.");
	}
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
		pDisplay = new SSD1306Wire(0x3c);//, SDA_Pin, SCL_Pin, GEOMETRY_128_64);
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






constexpr uint64_t sToMicroS(uint64_t seconds)
{
	return seconds * 1000 * 1000 /*uS_TO_S_FACTOR*/;
}

constexpr uint64_t milliToMicroS(uint64_t millis)
{
	return millis * 1000 /*uS_TO_mS_FACTOR*/;
}

constexpr uint64_t sToMilliS(uint64_t seconds)
{
	return seconds * 1000 /*mS_TO_S_FACTOR*/;
}


void Device::GotoDeepSleep() 
{
	Serial.println("Going to deep sleep.");
	SPI.end();
	adc_power_off();
	
	//from https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/issues/6#issuecomment-518896314
	pinMode(RST_LoRa,INPUT);  

	uint64_t sleepTimeUs = sToMicroS(config.TimeBetween);
	auto workTimeMs = status.millisFromStart();
	ESP_LOGI(TAG,"Work time %dms",workTimeMs);
	sleepTimeUs -=  milliToMicroS(workTimeMs); 
	esp_sleep_enable_timer_wakeup(sleepTimeUs);
	ESP_LOGI(TAG,"Going to deep sleep for %ds\n", sleepTimeUs/1000/1000);
	Serial.end();
	VextOFF();
	esp_deep_sleep_start();    
}



