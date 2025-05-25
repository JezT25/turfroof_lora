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

#include "system_node.hpp"

void IOT_class::Initialize()
{
	delay(CONN_DELAY);

	// Connect to Wi-Fi
	#ifdef DEBUGGING
		Serial.print(F("Connecting to "));
		Serial.println(config::SSID);
	#endif

	WiFi.begin(config::SSID, config::PASSWORD);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(CONN_DELAY);

		#ifdef DEBUGGING
			Serial.print('.');
		#endif
	}

	#ifdef DEBUGGING
		Serial.println();
		Serial.println(F("WiFi connected"));
		Serial.print(F("IP address: "));
		Serial.println(WiFi.localIP());
	#endif

	// Start NTP client
	timeClient.begin();
}

void IOT_class::getTime()
{
	timeClient.update();

	#ifdef DEBUGGING
		Serial.print(F("Current time: "));
		Serial.println(timeClient.getFormattedTime());
	#endif
}