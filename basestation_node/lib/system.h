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

#include "system_node.hpp"

class SYSTEM_class
{
	private:
		IDATA&              _IData;
		IOT_class&	        _iot;
		LORA_MODULE_class&  _lora_module;

    void Initialize_System();
    void clearData();
  
    #ifdef DEBUGGING
      void displayData();
    #endif

	public:
		SYSTEM_class(SystemComponents& class_lib) : _IData(class_lib._IData), _iot(class_lib._iot), _lora_module(class_lib._lora_module) {}

		void Initialize();
		void Run();
};

#endif
