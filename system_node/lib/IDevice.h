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

class IDATA
{
	public:

		uint8_t HW_ID	            =	  0;

		double SYSTEM_TEMPERATURE  =   0;

		double SYSTEM_HUMIDITY     =   0;

		double SOIL_TEMPERATURE    =   0;

		double SOIL_MOISTURE       =   0;

		double BATTERY_VOLTAGE     =   0;
};

#endif
