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

#include <cstdint>

#ifndef Config_hpp
#define Config_hpp

#define MAX_DEVICES		9

#define ACTIVE         	1
#define NOT_IN_USE     	0

namespace config
{
	// WiFi Credentials
	constexpr const char* SSID                       = "Vodafone-7zgp";
	constexpr const char* PASSWORD                   = "yrdCy2x4";

	// Basestation HW_ID
	constexpr uint8_t HW_ID                          = 8;

	// ACTIVE_DEVICES: Indicates which HW_IDs are active
	constexpr uint8_t ACTIVE_DEVICES[MAX_DEVICES] =
	{ 
		NOT_IN_USE, // HW_ID 0
		NOT_IN_USE, // HW_ID 1
		ACTIVE, 	// HW_ID 2
		ACTIVE,     // HW_ID 3
		NOT_IN_USE, // HW_ID 4
		ACTIVE,     // HW_ID 5
		NOT_IN_USE, // HW_ID 6
		ACTIVE,     // HW_ID 7

		// This is us and also for checksum, DO NOT USE!
		NOT_IN_USE  // HW_ID 8
	};
}

#endif