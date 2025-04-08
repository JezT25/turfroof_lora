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

#include "../system_node.hpp"

void SD_CARD_MODULE_class::Initialize(IDATA IData)
{
	_filename = String(IData.HW_ID) + FILENAME_EXTENSION;

	#ifdef DEBUGGING
		Serial.print("SD Card Log Filename: ");
		Serial.println(_filename);
	#endif
}

void SD_CARD_MODULE_class::logData(IDATA IData, time_t t)
{
	char datetime[20];
	snprintf(datetime, sizeof(datetime), "%04d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));

	#ifdef DEBUGGING
		Serial.print("Writing to SD Card: ");
		Serial.print(datetime);Serial.print(",");
		Serial.print(IData.SYSTEM_TEMPERATURE, DECIMAL_VALUES);Serial.print(",");
		Serial.print(IData.SYSTEM_HUMIDITY, DECIMAL_VALUES);Serial.print(",");
		Serial.print(IData.SOIL_TEMPERATURE, DECIMAL_VALUES);Serial.print(",");
		Serial.print(IData.SOIL_MOISTURE);Serial.print(",");
		Serial.println(IData.BATTERY_VOLTAGE);
	#endif

	#ifndef DEBUGGING
		if (!SD.begin(SD_CS)) return;

		File dataFile = SD.open(_filename, FILE_WRITE);

		if (dataFile)
		{
			if (dataFile.size() == 0)
			{
				dataFile.println("Date&Time,Temperature(C),Humidity(%),SoilTemperature(C),SoilMoisture(%),Battery(V)");
			}

			dataFile.print(datetime);dataFile.print(",");
			dataFile.print(IData.SYSTEM_TEMPERATURE, DECIMAL_VALUES);dataFile.print(",");
			dataFile.print(IData.SYSTEM_HUMIDITY, DECIMAL_VALUES);dataFile.print(",");
			dataFile.print(IData.SOIL_TEMPERATURE, DECIMAL_VALUES);dataFile.print(",");
			dataFile.print(IData.SOIL_MOISTURE);dataFile.print(",");
			dataFile.println(IData.BATTERY_VOLTAGE);
			
			dataFile.close();
		}
	#endif
}
