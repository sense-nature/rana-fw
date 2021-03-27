/*
 * web_ui.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: zdroyer
 */

#include "WebUI.h"
#include "FSWrapper.h"
#include "RanaDevice.h"
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


CustomAPWebUI::CustomAPWebUI(Device &dev):theDevice(dev), webServer(), htmlContent(dev)
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
	//fs::FS & currFS = FSWrapper::getFS(); 
	webServer.on("/", [this](){ this->serverRoot(); });
	webServer.on("/state", [this](){ this->serverRoot(); });
	
	//webServer.serveStatic("/",currFS,"index.htm");
	//webServer.serveStatic("/",currFS,"/index.htm");
	
	//webServer.serveStatic("/index",currFS,"/index.htm");	
	webServer.on("/config",[this](){this->onConfig();});
	webServer.on("/save_config",[this](){this->onSaveConfig();});

	//webServer.serveStatic("/generate_204",currFS,"config.htm");
	//webServer.serveStatic("/state",currFS,"/state.htm");
	//webServer.serveStatic("/state.htm",currFS,"/state.htm");

	webServer.on("/time",[this](){this->onTime();});
	webServer.on("/assign",[this](){this->onAssign();});
	//webServer.on("/probes",[this](){this->onProbes();});
	//webServer.on(("/wifi", webserver_wifi);

		//server.on(F("/values"), webserver_values);
	webServer.onNotFound([this](){this->defaultNotFound();});
	webServer.begin(80);
	ESP_LOGD(TAG, "WebServer started");
}



void CustomAPWebUI::onAssign()
{
	String innerHtmlBody =  webServer.arg("address")+" to be assigned to T#"+webServer.arg("T");
	theDevice.config.setProbeAddess( (uint8_t)atoi(webServer.arg("T").c_str()), webServer.arg("address").c_str());   

	theDevice.config.SaveConfig();
	theDevice.config.ShowConfig();
	innerHtmlBody+=R"( \n<br><a href="/">go to status</a>)";
	serve(innerHtmlBody);
}

void CustomAPWebUI::onConfig()
{
	serve(htmlContent.configInnerBody());
}
void CustomAPWebUI::onSaveConfig()
{
	String text("OnSaveConfig<br>\n");
	text.reserve(2000);
	for(int i=0; i< webServer.args(); i++){
	
		text+=webServer.argName(i);
		text+="=";
		text+=webServer.arg(i);
		text+="<br>\n";
	}
	String interval = webServer.arg("interval");
	interval.trim();
	int iInt = atoi(interval.c_str());
	if( iInt > 20 )
		theDevice.config.TimeBetween = iInt;
	else
		text +="Interval < 20s, not changed <br>";

	theDevice.config.setDevaddr(webServer.arg(theDevice.config.DEVADDR_name).c_str());
	theDevice.config.setNwkskey(webServer.arg(theDevice.config.NWKSKEY_name).c_str());
	theDevice.config.setAppskey(webServer.arg(theDevice.config.APPSKEY_name).c_str());
	theDevice.config.setSF((uint8_t)atoi(webServer.arg(theDevice.config.SF_name).c_str()));
	int iNumProbes = atoi(webServer.arg("number_of_probes").c_str());
	int iLastProbeIndex = theDevice.config.lastProbeIndex(); 
	

	theDevice.config.SaveConfig();
	theDevice.config.ShowConfig();

	serve(text);
}




void CustomAPWebUI::onTime() 
{
	String source = webServer.arg("source");
	source.trim();
	String timestamp = webServer.arg("timestamp");
	timestamp.trim();

	String content;
	content.reserve(1024);

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
	theDevice.ReadDS18B20Temperatures();
	serve(htmlContent.currentStateInnerBody());

/*
    String json = "{";
    json += "\"free_heap\":" + String(ESP.getFreeHeap())+";";
    json += "}";
    webServer.send(200, "text/json", json);
    json = String();
//*/
}

void CustomAPWebUI::serve(const String & innerHtmlBody)
{
	webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
	webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	webServer.send(200, "text/html", htmlContent.getPageTop());
	webServer.sendContent(innerHtmlBody);
	webServer.sendContent(htmlContent.getPageFooter());
}

void CustomAPWebUI::defaultNotFound()
{
	//webServer.sendHeader()
	webServer.sendContent("Error 404 - Page not found");
}

void CustomAPWebUI::runWebserver(const IPAddress & ip)
{
	ESP_LOGI(TAG,"Starting webserver on : %s",ip.toString().c_str());

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
	const char * defSP = "HappyFrog";
	bool apActive = false;
	const char * hostname ("sense-node");

	auto ch = selectChannelForAp();
	auto status =  WiFi.begin(defSP,defSP);
	for(int i=0; i<10 && status != WL_CONNECTED ; i++){
		ESP_LOGI(TAG, "Attempt #%d to connect to default WiFi, last state: %d",i,status);
		status =  WiFi.begin(defSP,defSP);
		delay(500);
	}
	if( status != WL_CONNECTED ){
		ESP_LOGI(TAG, "Could not connect to the default WiFi, starting of AP");

		WiFi.setHostname(hostname );
		const IPAddress apIP(10, 12, 14, 1);
		WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
		auto ssid = getAPSsid();
		
		if( ! WiFi.softAP( ssid.c_str(),  getAPPassword(), ch ) )
			ESP_LOGI(TAG,"Could not start WebUI AP");
		else {
			ESP_LOGI(TAG,"Starting AP wifi, ssid [%s], channel:%d ",ssid.c_str(), ch);
			apActive = true;
		}
	} else {
		ESP_LOGI(TAG,"Conncted to default WiFi ",defSP);
	}

	delay(2000); //let the soft AP start 
	// In case we create a unique password at first start
	//debug_outln_info(F("AP Password is: "), cfg::fs_pwd);
	if (MDNS.begin( hostname )) {
		MDNS.setInstanceName( "Config of the sensing node");
		MDNS.addService("_http", "_tcp", 80);
		MDNS.addServiceTxt("_http", "_tcp", "PATH", "/");
		ESP_LOGI(TAG, "mDNS begun " );
	}

	runWebserver( WiFi.localIP() );

	if( apActive ){
		WiFi.softAPdisconnect(true);
		ESP_LOGI(TAG,"WiFi AP finished");
	}

	return true;
}

