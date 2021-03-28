#include "RanaDevice.h"
#include "OWTemperatures.h"
#include "utils.h"
#include "WebUI.h"

#include <Arduino.h>
#include <driver/adc.h>
#include <esp_system.h>
#include <DallasTemperature.h>
#include <OneWire.h>    
#include <SPI.h>

#include <RtcDS3231.h>
#include <EepromAT24C32.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> 


using namespace Rana;


extern volatile bool Rana::staticKeyPressChange;


void IRAM_ATTR keyPressed() {
	staticKeyPressChange = true;
}



void Device::VextON(void)
{
    pinMode(Vext,OUTPUT);
    digitalWrite(Vext, LOW);
    delay(100u);
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

String Device::GetHWString()
{
	String ret(WiFi.macAddress());
	return ret;
}


void Device::StartDevice() 
{
    status.startUpNow();
	attachInterrupt(KEY_BUILTIN, keyPressed, CHANGE);
	Serial.begin(115200ul);
	ESP_LOGI(TAG, "\n--------------\n----------------\nRana start");
	Rana::Device::VextON();
	config.ReadConfig(status);
	config.ShowConfig();
	Wire.begin(SDA_Pin, SCL_Pin);
	Rana::Device::GetDisplay();
	ReadAndUpdateEepromData();
	ReadRTCTime();
	ReadDS18B20Temperatures();
	GetDisplay()->flipScreenVertically();
	GetDisplay()->drawString(0, 0, "# " + String(status.getBootCount())+" | "+String(status.measurementCount)); 
	GetDisplay()->drawString(0, 33," probes: " + String(status.unknownProbeTemperatures.size() +status.knownProbeTemperatures.size()));
	GetDisplay()->display();
	delay(1000);
	ReadInternalSensorValues();
	ReadBatteryLevel();
	status.SaveToSD();  
	FSWrapper::EndDSCardOps();

	if(status.enterConfigMode() ){
		startWebConfig();
	}
	sendMeasurementsOveLoRaWAN();
	ESP_LOGD(TAG, "At device setup end : free heap: %gKB",esp_get_free_heap_size()/1024.0);	
}


void Device::Loop() 
{
    lora.Loop();

}



void  Device::startWebConfig()
{
	ESP_LOGI("Starting web config");
	CustomAPWebUI webUI(*this);
	webUI.startWebUI();	
}

void  Device::sendMeasurementsOveLoRaWAN()
{
	ESP_LOGI("Sending the data over LoRaWAN");
	lora.initLoRaWAN(status.measurementCount, config);
	ESP_LOGD(TAG,"theDevice pointer: 0x%X",this);
	lora.sendData(*this);


}


void Device::ReadInternalSensorValues()
{
	Adafruit_BME280 bme;
	if( ! bme.begin(0x76, &Wire) )
		return;
	status.internalSensor = Status::SensorType::BME280;

	bme.setSampling(Adafruit_BME280::sensor_mode::MODE_FORCED
					, Adafruit_BME280::sensor_sampling::SAMPLING_X1
					, Adafruit_BME280::sensor_sampling::SAMPLING_X1
					, Adafruit_BME280::sensor_sampling::SAMPLING_X1
					,Adafruit_BME280::sensor_filter::FILTER_OFF);

	status.intTemperature = bme.readTemperature(); /* *C */
	status.intHumidity = roundf(bme.readHumidity()); /* % */
	status.intPressure = roundf(bme.readPressure() / 100.0F /* hPa */);
}

void Device::ReadDS18B20Temperatures()
{
	OWTemperatures::ReadValues(OneWire_Pin, config ,status);
}


void Device::ReadRTCTime()
{ 
	// defaut time set manually, used epochconverter.com
	// buildtimestamp is costly because it forces full recompilation
	// time_t defaultTimeUnix = 1616889600;  	// 2021-03-28 00:00:00 UTC
	const RtcDateTime defaultDT = rdtFromTimestamp( 1616889600 );	

	ESP_LOGI(TAG,"Default timestamp UTC date: %s", rtcDTToString(defaultDT).c_str() );
	RtcDS3231<TwoWire> rtc(Wire);
	rtc.Begin();
	if( ! rtc.IsDateTimeValid() ){
		if( rtc.LastError() != 0 ){
			ESP_LOGE(TAG, "RTC I2C communication error %d",rtc.LastError() );
			status.rtc = Status::RTCState::NoRTC;
		} else { 
			ESP_LOGE(TAG, "RTC low battery or time not set in the RTC");
		}
	} 
	if( rtc.LastError() == 0 ) {
		if( ! rtc.GetIsRunning() ){
			ESP_LOGI(TAG,"RTC not running -> setting to build timestamp + 1 minute");
			status.rtc = Status::RTCState::RTC_NOT_RUNNING;
		} else {
			RtcDateTime now = rtc.GetDateTime();
    		if (now <  defaultDT ) {
				rtc.SetDateTime(defaultDT);
				ESP_LOGI(TAG,"RTC time earlier than the default time -> setting to the default timestamp");
				status.rtc = Status::RTCState::TIME_SET_FROM_DEFAULT_TIME;
			} else {
				status.rtc = Status::RTCState::RTC_OK;
			}
		}	
		status.rtcUtcTime = rtc.GetDateTime();
		ESP_LOGI(TAG,"Current ExtRTC time:  %s", rtcDTToString(status.rtcUtcTime).c_str());
		rtc.Enable32kHzPin(false);
    	rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
	}

}
void Device::ReadAndUpdateEepromData()
{
	EepromAt24c32<TwoWire> eeprom(Wire);
	eeprom.Begin();
	const uint16_t valueAddress = 8;
	ESP_LOGD(TAG,"Current boot count: %u, measurement count: %u", staticBootCount, status.measurementCount);
	uint32_t eepromValue = 0;
	auto res = eeprom.GetMemory(valueAddress, (uint8_t*)&eepromValue, sizeof(eepromValue));
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
void Device::ReadBatteryLevel() 
{
    //works only on the older moduled, chipid xxxx 5B1DA0D8 , not the v2.1

    //adc_power_on();
    //posssibly adc should be here initialized            
	delay(100u);
    pinMode(Battery_Pin,OPEN_DRAIN);
    delay(100u);
	status.batteryLevel = analogRead(Battery_Pin);
	ESP_LOGD(TAG,"Read battery level %u",status.batteryLevel);
    //batteryVoltage = analogRead(Battery_Pin); 
    //Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
	//Serial.println("Battery voltage reading: "+ String(batteryVoltage));
	pinMode(Battery_Pin, OUTPUT);
	digitalWrite(Battery_Pin, LOW);      
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
	adc_power_off();
	
	//from https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/issues/6#issuecomment-518896314
	pinMode(RST_LoRa,INPUT);
	SPI.end();


/*
	pinMode(KEY_BUILTIN, INPUT_PULLDOWN);
	esp_sleep_enable_ext0_wakeup((gpio_num_t)KEY_BUILTIN, 1);
	rtc_gpio_pullup_dis((gpio_num_t)KEY_BUILTIN);
	rtc_gpio_pulldown_en((gpio_num_t)KEY_BUILTIN);
*/
	VextOFF();

	uint64_t sleepTimeUs = sToMicroS(config.TimeBetween);
	uint64_t workTimeMs = status.millisFromStart();
	ESP_LOGI(TAG,"Work time %ums",workTimeMs);
	uint64_t workTimeUs = milliToMicroS(workTimeMs);

	if( workTimeUs > sleepTimeUs){
		ESP_LOGI(TAG,"Work time longer than  sleep time, skipping %d cycles",workTimeUs/sleepTimeUs);
		workTimeUs %= sleepTimeUs;
	}

	sleepTimeUs -=  workTimeUs; 
	esp_sleep_enable_timer_wakeup(sleepTimeUs);
	ESP_LOGI(TAG,"Going to deep sleep for %us\n", sleepTimeUs/1000/1000);
	Serial.flush();
	Serial.end();
	esp_deep_sleep_start();    
}



