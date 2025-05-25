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

		float MODULE_0_DATA		=	0;
		
		float MODULE_1_DATA		=	0;
		
		float MODULE_2_DATA		=	0;
		
		float MODULE_3_DATA		=	0;
		
		float MODULE_4_DATA		=	0;
		
		float MODULE_5_DATA		=	0;
		
		float MODULE_6_DATA		=	0;

		float MODULE_7_DATA		=	0;
};

#endif
