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

#ifndef sd_card_module_h
#define sd_card_module_h

#include "../system_node.hpp"

#define LOGGING_BAUD  9600
#define LOGGING_DELAY 1500

// OpenLog Config Settings
// 9600,26,3,1,1,1,0
// baud,escape,esc#,mode,verb,echo,ignoreRX

class SD_CARD_MODULE_class
{
	public:
		void logData(IDATA IData, time_t t);

};

#endif
