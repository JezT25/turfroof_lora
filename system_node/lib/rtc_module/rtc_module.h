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

#ifndef rtc_module_h
#define rtc_module_h

#include "../system_node.hpp"

// DS3231 Pins
#define RTC_INT			3

#define ALARM1_MIN		0
#define ALARM2_MIN		5

#define NO_TRIGGER		0
#define ALARM1_TRIGGER	1
#define ALARM2_TRIGGER	2

class RTC_MODULE_class
{
    private:
    	DS3232RTC _rtc;
		
		#ifdef DEBUGGING
			void printtimedate(time_t t);
			void settimefromPC();
		#endif

    public:
        bool Initialize();
        bool Sync();
        uint8_t checkAlarm();
};

#endif
