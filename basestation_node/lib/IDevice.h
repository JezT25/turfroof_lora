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

#ifndef IDevice_h
#define IDevice_h

#include "system_node.hpp"

class IDATA
{
	public:

		float TEMP_DATA[MAX_DEVICES]	=	{ 0 };

		float HUMI_DATA[MAX_DEVICES]	=	{ 0 };

		float STMP_DATA[MAX_DEVICES]	=	{ 0 };

		float SMOI_DATA[MAX_DEVICES]	=	{ 0 };

		float BATT_DATA[MAX_DEVICES]	=	{ 0 };
};

#endif
