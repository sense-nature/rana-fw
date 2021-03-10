/*
 * web_ui.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: zdroyer
 */

#include "WebUI.h"
#include "FSWrapper.h"
#include "utils.h"
#include "rana_logging.h"


#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include <Arduino.h>
#include <Wire.h> 
#include <RtcDS3231.h>
#include <EepromAT24C32.h>


#define sApSsidPrefix "WebUI "
#define sDefaultApPassword nullptr


using namespace Rana;


CustomAPWebUI::CustomAPWebUI():webServer()
{

}

CustomAPWebUI::~CustomAPWebUI(){}

String CustomAPWebUI::getAPSsid() const
{
	String s = sApSsidPrefix + WiFi.macAddress();
	ESP_LOGD(TAG, "Uniqie SSID for the AP: %s", s.c_str()) ;
	return s;
}

const char *  CustomAPWebUI::getAPPassword() const
{
	return sDefaultApPassword;
}

std::vector<wifiInfo_t> CustomAPWebUI::getAvailabelWifiAPs() const {
	WiFi.disconnect(true);
	ESP_LOGD(TAG, "scan for wifi networks...");
	std::vector<wifiInfo_t> wifiInfos;
	int8_t scanReturnCode = WiFi.scanNetworks(false /* scan async */, true /* show hidden networks */);
	if (scanReturnCode < 0) {
		ESP_LOGD(TAG, "WiFi scan failed. Treating as empty. ");
	} else {
		ESP_LOGD(TAG, "Scan for wifi networks...");
		uint8_t count_wifiInfo = (uint8_t) scanReturnCode;
		std::vector<wifiInfo_t>(count_wifiInfo).swap(wifiInfos);
		for (uint8_t i = 0; i < count_wifiInfo; i++) {
			//String SSID;
			uint8_t* BSSID;
			auto & info = wifiInfos[i];
			if(!WiFi.getNetworkInfo(i, info.ssid, info.encryptionType, info.RSSI, BSSID, info.channel )){
				ESP_LOGE(TAG, "Cannot get wifi info for network #%d",i);
			} else {
				ESP_LOGD(TAG,"#%d %s",i,info.toString().c_str());
			}
		}
	}
	return wifiInfos;

}

int CustomAPWebUI::selectChannelForAp() const {
	auto wifiInfos = getAvailabelWifiAPs();
	std::array<int, 14> channels_rssi;
	std::fill(channels_rssi.begin(), channels_rssi.end(), -100);
	for(const auto & info : wifiInfos){
		if( info.channel >=0 && info.channel < channels_rssi.size() ){
			if (info.RSSI > channels_rssi[info.channel]) {
				channels_rssi[info.channel] = info.RSSI;
			}
		}
	}
	if ((channels_rssi[1] < channels_rssi[6]) && (channels_rssi[1] < channels_rssi[11])) {
		return 1;
	} else if ((channels_rssi[6] < channels_rssi[1]) && (channels_rssi[6] < channels_rssi[11])) {
		return 6;
	} else {
		return 11;
	}
}

void CustomAPWebUI::setupWebserver()
{
	bool success = false;
	fs::FS & sd = *(FSWrapper::getSDFS(success)); 
	//webServer.on("/", [this](){ this->serverRoot(); });
	//webServer.serveStatic("/",sd,"index.htm");
	webServer.serveStatic("/",sd,"/index.htm");
	webServer.serveStatic("/index",sd,"/index.htm");	
	webServer.serveStatic("/config",sd,"/config.htm");
	webServer.serveStatic("/config.htm",sd,"/config.htm");
	webServer.serveStatic("/generate_204",sd,"config.htm");
	webServer.serveStatic("/state",sd,"/state.htm");
	webServer.serveStatic("/state.htm",sd,"/state.htm");

	webServer.on("/time",[this](){this->onTime();});
		//server.on(F("/config"), webserver_config);
		//server.on(F("/wifi"), webserver_wifi);
		//server.on(F("/values"), webserver_values);
	webServer.onNotFound([this](){this->defaultNotFound();});
	webServer.begin(80);
	ESP_LOGD(TAG, "WebServer started");
}





void CustomAPWebUI::onTime() 
{
	const char * srcParamName = "source";
	String source("");
	String content;

	const char * tsParamName = "timestamp";
	String timestamp;
	//if(webServer.hasArg(srcParamName))
	{
	
		for(int i = 0; i< webServer.args(); i++)
		{
			content+= webServer.argName(i);
			content+= "=";
			content+= webServer.arg(i);
			content+="<br> \n\r";
			if(webServer.argName(i).equalsIgnoreCase(srcParamName)){
				source = webServer.arg(i);
				source.trim();
			}
			if(webServer.argName(i).equalsIgnoreCase(tsParamName)){
				timestamp = webServer.arg(i);
				timestamp.trim();	
			}
		}
	}
	RtcDS3231<TwoWire> rtc(Wire);
	rtc.Begin();
	if( ! rtc.IsDateTimeValid() ){
		if( rtc.LastError() != 0 ){
			content += (String("RTC I2C communication error=")+rtc.LastError());
		} else { 
			content += "RTC low battery or time not set in the RTC";
		}
	} else {
		if( rtc.LastError() == 0 ) {
			RtcDateTime now = rtc.GetDateTime();			
			content += rtcDTToString(now);

		}
	}

	if(!source.isEmpty()){
		if(source.equalsIgnoreCase("browser") && !timestamp.isEmpty() ){
			ESP_LOGD(TAG,"Received browser ts=%s", timestamp.c_str());
			time_t ts = (time_t)atoll(timestamp.c_str());
			ESP_LOGD(TAG,"TS: %d", ts);
			RtcDateTime rdt = rdtFromTimestamp(ts);
			rtc.SetDateTime(rdt);
			content += ("\n\rRTC set to [" + rtcDTToString(rdt) + "] based on browser time");
		}
	} 

	webServer.sendContent(content);
}


void CustomAPWebUI::serverRoot()
{
    String json = "{";
    json += "\"free_heap\":" + String(ESP.getFreeHeap())+";";
    json += "}";
    webServer.send(200, "text/json", json);
    json = String();
}

void CustomAPWebUI::defaultNotFound()
{
	//webServer.sendHeader()
	webServer.sendContent("def 404 - Page not found");
}

void CustomAPWebUI::runWebserver(const IPAddress & ip)
{
	
	DNSServer dnsServer;
	// Ensure we don't poison the client DNS cache
	dnsServer.setTTL(0);
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(53, "*", ip);		
	ESP_LOGD(TAG, "DNS server started");					// 53 is port for DNS server


	setupWebserver();

	// 10 minutes timeout for wifi config
	const unsigned long timeout =  10 * 60 * 1000; //[ms]
	auto last_page_load = millis();
	while ((millis() - last_page_load) < timeout + 500) {
		dnsServer.processNextRequest();
		webServer.handleClient();
		yield();
	}
	ESP_LOGI(TAG, "Webserver timeout - end of serving, moving to sensing");
}

bool CustomAPWebUI::startAPWebUI()
{
	auto ch = selectChannelForAp();
	const char * hostname ("sense-node");  
	WiFi.setHostname(hostname );
	if (MDNS.begin( hostname )) {
		MDNS.setInstanceName( "Config of the sensing node");
		MDNS.addService("_http", "_tcp", 80);
		MDNS.addServiceTxt("_http", "_tcp", "PATH", "/");
		ESP_LOGI(TAG, "mDNS begun " );
	}

	const IPAddress apIP(10, 12, 14, 1);
	WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
	auto ssid = getAPSsid();

	if( ! WiFi.softAP( ssid.c_str(),  getAPPassword(), ch ) )
		ESP_LOGI(TAG,"Could not start WebUI AP");
	else 
		ESP_LOGI(TAG,"Starting AP wifi, ssid [%s], channel:%d ",ssid.c_str(), ch);
	delay(2000); //let the soft AP start 
	// In case we create a unique password at first start
	//debug_outln_info(F("AP Password is: "), cfg::fs_pwd);

	runWebserver( apIP );
	
	WiFi.softAPdisconnect(true);
	ESP_LOGI(TAG,"WiFi AP finished");

	return true;
}

