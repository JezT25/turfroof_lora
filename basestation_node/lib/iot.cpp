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

void IOT_class::waitUntilQueryTime()
{
	while (true)
	{
		timeClient.update();

		#ifdef DEBUGGING
			Serial.print(F("Current time: "));
			Serial.println(timeClient.getFormattedTime());
		#endif

		int currentMinute = timeClient.getMinutes();
		if (currentMinute == MINUTE_BREAK)
		{
			delay(SECOND_BREAK);
			break;
		}

		int minutesLeft = (MINUTE_BREAK - currentMinute + 60) % 60;
		if (minutesLeft >= THRESH_MIN_LONG)
		{
			delay(NTP_UPDATE_MS_LONG);
		}
		else if (minutesLeft >= THRESH_MIN_MID)
		{
			delay(NTP_UPDATE_MS_MID);
		}
		else if (minutesLeft >= FINAL_THRESH_MIN_SHORT)
		{
			delay(NTP_UPDATE_MS_SHORT);
		}
		else
		{
			delay(NTP_UPDATE_MS_FINAL);
		}
	}
}

void IOT_class::getTime()
{
	timeClient.update();

	#ifdef DEBUGGING
		Serial.print(F("Current time: "));
		Serial.println(timeClient.getFormattedTime());
	#endif
}

void IOT_class::uploadData(IDATA IData)
{
	char url[MAX_URL_LEN];
    WiFiClient client;

	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		if(config::ACTIVE_DEVICES[i] == ACTIVE)
		{
			snprintf(url, MAX_URL_LEN,
                "http://api.thingspeak.com/update?api_key=%s&field1=%.5f&field2=%.5f&field3=%.5f&field4=%u&field5=%.5f",
                config::THINGSPEAK_API_KEYS[i],
                IData.TEMP_DATA[i],
                IData.HUMI_DATA[i],
                IData.STMP_DATA[i],
                IData.SMOI_DATA[i],
                IData.BATT_DATA[i]);

            HTTPClient http;
            http.begin(client, url);
            int httpResponseCode = http.GET();

			#ifdef DEBUGGING
				Serial.println(F("Uploading Data to ThingSpeak!"));
				Serial.print(F("Device "));
				Serial.print(i);
				Serial.print(F(" Status | "));

				if (httpResponseCode > 0)
				{
					Serial.print(F("ThingSpeak Response Code: "));
					Serial.println(httpResponseCode);
				}
				else
				{
					Serial.print(F("Error sending to ThingSpeak: "));
					Serial.println(http.errorToString(httpResponseCode));
				}
			#endif

			http.end();
		}
	}
}
