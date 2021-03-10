/*
 * web_ui.h
 *
 *  Created on: Nov 5, 2020
 *      Author: zdroyer
 */

#ifndef WEB_UI_H_
#define WEB_UI_H_


#include <string>
#include <vector>
#include <DNSServer.h>
#include <WebServer.h>





#define LEN_WLANSSID 35



namespace Rana{

struct wifiInfo_t {
	String ssid="";
	uint8_t encryptionType;
	int32_t RSSI;
	int32_t channel;

	String encryption() const
	{
			switch(encryptionType){
			case WIFI_AUTH_OPEN:
				return F("OPEN");   //		 WIFI_AUTH_OPEN = 0,         /**< authenticate mode : open */
			case WIFI_AUTH_WEP:
				return F("WEP");  /**< authenticate mode : WEP */
			case WIFI_AUTH_WPA_PSK:          /**< authenticate mode : WPA_PSK */
			case WIFI_AUTH_WPA2_PSK:         /**< authenticate mode : WPA2_PSK */
			case WIFI_AUTH_WPA_WPA2_PSK:     /**< authenticate mode : WPA_WPA2_PSK */
			case WIFI_AUTH_WPA2_ENTERPRISE:  /**< authenticate mode : WPA2_ENTERPRISE */
				return F("WPA*");
			}
			return F("UNKNOWN ENCRYPTION");
		}

	String toString() const
	{
		return ssid + F(" ") + encryption()+ F(", Ch:") +channel+F(", RSSI:")+RSSI;
	}


};


class CustomAPWebUI
{
protected:
	WebServer webServer;

public:
	CustomAPWebUI();
	virtual ~CustomAPWebUI();

	bool startAPWebUI();
	void serverRoot();
	void defaultNotFound();

	void onTime();

protected:
	virtual String getAPSsid() const;
	virtual const char * getAPPassword() const;
	virtual int selectChannelForAp() const;
	virtual std::vector<wifiInfo_t> getAvailabelWifiAPs() const;

	virtual void setupWebserver();
	virtual void runWebserver(const IPAddress & ip);



};



}


#endif /* WEB_UI_H_ */
