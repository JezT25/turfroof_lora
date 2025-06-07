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
	constexpr const char* SSID                       = "Turfroof Wifi";
	constexpr const char* PASSWORD                   = "turfroofhaskoliislands";

	// Basestation HW_ID
	constexpr uint8_t HW_ID                          = 8;

	// ACTIVE_DEVICES: Indicates which HW_IDs are active
	constexpr uint8_t ACTIVE_DEVICES[MAX_DEVICES] =
	{ 
		NOT_IN_USE, // HW_ID 0
		ACTIVE, 	// HW_ID 1
		NOT_IN_USE, // HW_ID 2
		ACTIVE,     // HW_ID 3
		NOT_IN_USE, // HW_ID 4
		ACTIVE,     // HW_ID 5
		NOT_IN_USE, // HW_ID 6
		ACTIVE,     // HW_ID 7

		// This is US and also for checksum, WARNING: DO NOT USE!
		NOT_IN_USE  // HW_ID 8
	};

	// API Write Keys for ThingSpeak
	constexpr const char* THINGSPEAK_API_KEYS[MAX_DEVICES] =
	{ 
		"RESERVED",                    // HW_ID 0
		"H3DOMD0C33D9ARBO",            // HW_ID 1
		"RESERVED",                    // HW_ID 2
		"PKS5JHFL7RRUN92K",            // HW_ID 3
		"RESERVED",                    // HW_ID 4
		"WAG41QFEJSYAO9RP",            // HW_ID 5
		"RESERVED",                    // HW_ID 6
		"NP59JEAYHD66BAD6",            // HW_ID 7

		// This is US and also for checksum, WARNING: DO NOT USE!
		"RESERVED"                     // HW_ID 8
	};
}

#endif