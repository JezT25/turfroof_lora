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

#define NTP_SERVER      "pool.ntp.org"
#define UTC_OFFSET      0           	// UTC+0 for Iceland (in seconds)
#define NTP_UPDATE_MS   60000       	// Update every 60 seconds

#define CONN_DELAY		500

class IOT_class
{
	private:
		WiFiUDP ntpUDP;
		NTPClient timeClient;

	public:
		IOT_class() : timeClient(ntpUDP, NTP_SERVER, UTC_OFFSET, NTP_UPDATE_MS) {}

		void Initialize();
		void getTime();
};

#endif
