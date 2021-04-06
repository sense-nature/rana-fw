#define LMIC_DEBUG_LEVEL 4 

#include <pins_arduino.h>   
#include <lmic.h>
#include <hal/hal.h> 

#include "RanaDevice.h"
#include "LoRaWANConn.h"

#include "rana_logging.h"



using namespace Rana;



void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }    



const lmic_pinmap lmic_pins = {
    .nss = SS, //18
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RST_LoRa,
    .dio = {DIO0,  DIO1 , DIO2}  //26, 35, 34 
}; 

dr_t GetSF(uint8_t uintSF)
{
	if( uintSF >= 12  )
		return DR_SF12;
	if( uintSF <= 7 )
		return DR_SF7;
	switch (uintSF)
	{
	case 11: return DR_SF11;
	case 10: return DR_SF10;
	case 9: return DR_SF9;
	case 8: return DR_SF8;
	case 7: return DR_SF7;
	}
	return DR_SF7;
}


 void LMIC_ABI_STD LoRaWANCallback(void *pUserData, ev_t e)
 {
	 static const char * ev_names[] = {LMIC_EVENT_NAME_TABLE__INIT};
	 ESP_LOGD(TAG,"LMIC event: %s",ev_names[e]);
	Rana::Device * pDevice = ((Rana::Device *)pUserData);

	 switch (e)
	 {
	 case EV_JOIN_FAILED:
	 case EV_REJOIN_FAILED:
	 	ESP_LOGE(TAG,"LoRaWAN (RE)JOIN FAILED, deep sleep now");
		pDevice->GotoDeepSleep();	 
		 break;
	case EV_TXCANCELED:
	 	ESP_LOGE(TAG,"LoRaWAN transmit cancelled, deep sleep now");
		pDevice->GotoDeepSleep();	 
	 default:
		 break;
	 }
 }

LoRaWANConn::LoRaWANConn() 
{
    
}

LoRaWANConn::~LoRaWANConn() 
{
    
}


void LoRaWANConn::initLoRaWAN(Rana::Device &device) {
    

	// LMIC init
	os_init_ex(&lmic_pins);
	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();
	LMIC_setClockError(MAX_CLOCK_ERROR * 3 / 100);

	// Set static session parameters. TTN network has id 0x13
	LMIC_setSession((unsigned)0x13, device.config.DEVADDRu32(), device.config.NWKSKEY, device.config.APPSKEY);

	LMIC_setSeqnoUp(device.status.measurementCount);
	//(bootCount);
	// Set up the channels used by the Things Network, which corresponds
	// to the defaults of most gateways. Without this, only three base
	// channels from the LoRaWAN specification are used, which certainly
	// works, so it is good for debugging, but can overload those
	// frequencies, so be sure to configure the full frequency range of
	// your network here (unless your network autoconfigures them).
	// Setting up channels should happen after LMIC_setSession, as that
	// configures the minimal channel set.
	///*
	 LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
	 LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(EU868_DR_SF12, EU868_DR_SF7B), BAND_CENTI);      // g-band
	 LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
	 LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
	 LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
	 LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
	 LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
	 LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
	 LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(EU868_DR_FSK,  EU868_DR_FSK),  BAND_MILLI);      // g2-band

	 //*/
	// TTN defines an additional channel at 869.525Mhz using SF9 for class B
	// devices' ping slots. LMIC does not have an easy way to define set this
	// frequency and support for class B is spotty and untested, so this
	// frequency is not configured here.
	// Disable link check validation
	LMIC_setLinkCheckMode(0);
	// TTN uses SF9 for its RX2 window.
	LMIC.dn2Dr = DR_SF9;
	// Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
	//LMIC_setDrTxpow(DR_SF11,14);
	//LMIC_setDrTxpow(DR_SF9,14);

	LMIC_registerEventCb(LoRaWANCallback,(void*)(&device));
	LMIC_setDrTxpow(GetSF(device.config.SF), 14);
	LMIC_startJoining();
	
	ESP_LOGD(TAG,"LMIC after start joining");
}                                     



void LoRaWANConn::Loop() 
{
//	ESP_LOGD(TAG,"LoRaWANConn::Loop");
    os_runloop_once();     
}


uint8_t getHigh(int16_t val){

	return (val & 0xFF00) >> 8;
}

uint8_t getHigh(uint16_t val){
	return (val & 0xFF00) >> 8;
}


int8_t getLow(int16_t val){
	return val & 0xFF;
}
uint8_t getLow(uint16_t val){
	return val & 0xFF;
}

/*
void push2BytesToMessage(std::vector<uint8_t> & vect, int16_t iValue){
	vect.push_back(getHigh(iValue));
	vect.push_back(getLow(iValue));
}
//*/


void push2BytesToMessage(uint8_t * buffer, uint8_t &firstFreeIndex, int16_t iValue){
	buffer[firstFreeIndex++] = getHigh(iValue);
	buffer[firstFreeIndex++] = getLow(iValue);
}


/*
void pushTemperatureToMessage(std::vector<uint8_t> & vect, float fTemperature){
	int16_t iTemp = (int16_t)roundf(fTemperature*100.0f);
	push2BytesToMessage(vect, iTemp);
}
//*/
void pushTemperatureToMessage(uint8_t * buffer, uint8_t &firstFreeIndex, float fTemperature)
{
	int16_t iTemp = (int16_t)roundf(fTemperature*100.0f);
	push2BytesToMessage(buffer,firstFreeIndex, iTemp);
}

    
void pushByteToMessage(uint8_t * buffer, uint8_t &firstFreeIndex, uint8_t value)
{
	buffer[firstFreeIndex++] = value;	
	
}




void afterLoraPacketSent(void *pUserData, int fSuccess){
// Heltec.display->clear();
	ESP_LOGD(TAG,"pUserData pointer: 0x%X",pUserData);
    ESP_LOGD(TAG,"After Lora Packet sent");
	Rana::Device * pDevice = ((Rana::Device *)pUserData);
    if( fSuccess){
        pDevice->GetDisplay()->drawString (0,35, " SENT");
        ESP_LOGI(TAG,"Sending result: OK");
    } else {
        pDevice->GetDisplay()->drawString (0, 36," FAILED");
        ESP_LOGI(TAG,"Sending result: FAILED");
    }
	pDevice->GetDisplay()->display();
    //ESP_LOGI(TAG,"Sleep before the end ");
	//delay(5000);
    ESP_LOGI(TAG,"End of processing");

    pDevice->GotoDeepSleep();
}             

void LoRaWANConn::sendData(Rana::Device &device) 
{
	initLoRaWAN(device);
	ESP_LOGD("Start the message assembly");
	//buffer for [Byte] 
	// statuses: 2
	//battery: 2
	//internal climate: 3
	//8 * probe temps: 8*2
	static uint8_t buffer[ 2 + 2 + 3 + 8*2 ] = {};
	uint8_t currIdx = 0;

	pushByteToMessage(buffer,currIdx,device.status.getSessionStatus());
	pushByteToMessage(buffer,currIdx,device.status.getProbesStatus());
	push2BytesToMessage(buffer,currIdx,device.status.batteryLevel);
    
	if(device.status.hasInternalSensor()){
		pushByteToMessage(buffer,currIdx,(uint8_t)roundf(device.status.intHumidity));
		pushTemperatureToMessage(buffer,currIdx,device.status.intTemperature);
	}
	uint8_t n=0;
    for(uint8_t i=0 ; n<device.status.knownProbeTemperatures.size() && i<8; i ++){
		auto it = device.status.knownProbeTemperatures.find(i);
		if( it!= device.status.knownProbeTemperatures.end()){
			pushTemperatureToMessage(buffer,currIdx,it->second.second);
			n++;
		}
	}
    // Prepare upstream data transmission at the next possible time.
	ESP_LOGD(TAG,"Prepared message is %u B: %s, ",currIdx, binToHexString(buffer, currIdx).c_str());
    lmic_tx_error_t ret = LMIC_sendWithCallback( device.config.NodeNumber, buffer, currIdx, 0, afterLoraPacketSent, (void*)(&device));
    if( ret != LMIC_ERROR_SUCCESS ){
        ESP_LOGE(TAG,"Cannot register sending the LoRaWAN package. Error = %d",ret);
    } else {
        device.LedON();
        ESP_LOGI(TAG,"Sending uplink packet #%d", device.status.measurementCount);
    }
}



// void do_send(osjob_t* j, void(*callBack)(void *, int)){

// 	if( firstRun() ){
// 		//don't send anything once in the first  after turning on
// 		LedON();
// 		Serial.println("Skipping sending packet #"+ String (bootCount));
// 		goToDeepSleep();
// 	} else	{
// 		// Payload to send (uplink)
// 		uint8_t serialNo = DEVICE_ID;
// 		std::vector<uint8_t> message;
// 		message.push_back(sessionStatus);
// 		push2BytesToMessage(message, batteryVoltage);
// 		message.push_back(boxHumidity);
// 		push2BytesToMessage(message, boxPressure);
// 		pushTemperatureToMessage(message, boxTemperature);
// 		for(int i=0 ; i< N_TEMP; i++)
// 			pushTemperatureToMessage(message, temp[i]);

// 		// Prepare upstream data transmission at the next possible time.
// 		lmic_tx_error_t ret = LMIC_sendWithCallback(serialNo, message.data(), message.size(), 0, callBack, (void*)0);
// 		if( ret != LMIC_ERROR_SUCCESS ){
// 			Serial.println("Cannot register sending the LoRaWAN package. Error = "+String(ret));
// 		} else {
// 			LedON();
// 			Serial.println("Sending uplink packet #"+ String (bootCount));
// 		}
// 	}
// }    

