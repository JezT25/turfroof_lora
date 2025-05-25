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

namespace config
{
    constexpr const char* SSID 	 					= "Vodafone-7zgp";
    constexpr const char* PASSWORD 					= "yrdCy2x4";
    constexpr uint8_t HW_ID 						= 8;
    // constexpr uint8_t ACTIVE_DEVICES[MAX_DEVICES]	= { 0, 1, 0, 1, 0, 1, 0, 1, 0 };
    constexpr uint8_t ACTIVE_DEVICES[MAX_DEVICES]	= { 0, 1, 0, 1, 0, 0, 0, 0, 0 };
}

#endif