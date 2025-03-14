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
        
		float SYSTEM_TEMPERATURE   =   11.111;

		float SYSTEM_HUMIDITY      =   99.999;

		float SOIL_MOISTURE        =   55.555;
};

class ISYSTEM
{
    public:

        uint8_t HW_ID   =   0;
};

#endif
