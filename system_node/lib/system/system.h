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

#ifndef system_h
#define system_h

#include "../system_node.hpp"

#define BATTERY_LEVEL_CUTOFF	3.25000
#define DELAY_SMALL 			50

class SYSTEM_class
{
	private:
		IDATA&                  _IData;
		HWIO_class&             _hwio;
		RTC_MODULE_class&       _rtc_module;
		LORA_MODULE_class&      _lora_module;
		SD_CARD_MODULE_class&   _sd_card_module;

		static volatile bool _interruptbyLoRa;
		static volatile bool _interruptbyRTC;

		static inline void wakeonRTC();
		static inline void wakeonLoRa();
		inline void gotosleep();
		inline void isBatteryLevelSufficient();
		void entersleepMode();
		void enterlightsleepMode();

		#ifdef DEBUGGING
			uint16_t freeRAM();
			void displayfreeRAM();
		#endif

	public:
		SYSTEM_class(SystemComponents& class_lib) : _IData(class_lib._IData), _hwio(class_lib._hwio), _rtc_module(class_lib._rtc_module), _lora_module(class_lib._lora_module) , _sd_card_module(class_lib._sd_card_module){}
		
		void Initialize();
		void Run();
};

#endif
