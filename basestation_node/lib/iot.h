/*
	============================================================
	Master's Thesis in Electrical and Computer Engineering
	Faculty of Electrical and Computer Engineering
	School of Engineering and Natural Sciences, University of Iceland

	Title: Design and Implementation of a Low-Power LoRa Mesh Sensor Network 
				 for Monitoring Soil Conditions on Icelandic Turf Roofs

	Researcher: Jezreel Tan
	Email: jvt6@hi.is

	Supervisors:
	Helgi Þorbergsson
	Email: thorberg@hi.is

	Dórótea Höeg Sigurðardóttir
	Email: dorotea@hi.is

	============================================================
*/

#ifndef iot_h
#define iot_h

#include "system_node.hpp"

#define MAX_URL_LEN				255

#define NTP_SERVER      		"pool.ntp.org"
#define UTC_OFFSET      		0           	// UTC+0 for Iceland (in seconds)

#define MINUTE_BREAK			0
#define SECOND_BREAK			30000

#define THRESH_MIN_LONG			20
#define THRESH_MIN_MID	 		10
#define FINAL_THRESH_MIN_SHORT	2

#define NTP_UPDATE_MS_LONG  	900000       	// Update every 15 minutes
#define NTP_UPDATE_MS_MID  		300000     		// Update every 5 minutes
#define NTP_UPDATE_MS_SHORT		60000      		// Update every minute
#define NTP_UPDATE_MS_FINAL		1000			// Update every second

#define CONN_DELAY				500

class IOT_class
{
	friend class LORA_MODULE_class;

	private:
		WiFiUDP ntpUDP;
		NTPClient timeClient;

	public:
		IOT_class() : timeClient(ntpUDP, NTP_SERVER, UTC_OFFSET, NTP_UPDATE_MS_FINAL) {}

		void Initialize();
		void getTime();
		void waitUntilQueryTime();
		void uploadData(IDATA IData);
};

#endif
