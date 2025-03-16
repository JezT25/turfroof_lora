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

#include "../system_node.hpp"

void SYSTEM_class::Initialize()
{
    // Initialize Hardware
    _hwio.Initialize(&_ISystem);

    // Initialize Lora Module
    _lora_module.Initialize();
}

void SYSTEM_class::Run()
{
	// Device is always sleeping here

	{
	// lora interrupt wakes device

	// do the lora stuff
	_lora_module.getMessages(_IData, _ISystem);

	// log in sd card
	}

	{
	// clock interrupt wakes device
	// log in sd card
	}

	// go back to sleep
}