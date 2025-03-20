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
	_lora_module.Initialize(_ISystem);

	#ifdef DEBUG_ON
		Serial.print("System Version: v");
		Serial.println(SYSTEM_VER);
		Serial.println(".........................\n");
	#endif
}

void SYSTEM_class::Run()
{
	// TODO: BUY
	// buy ds18b20 temp sensor
	// sht31 for humidity outside
	// soil moisture sensor
	// battery module


	// Device is always sleeping here

	{
	// lora interrupt wakes device
	// get details
	// do the lora stuff
	// while not moving for 2 minute finish lora stuff
	_lora_module.loadSensorData(_IData);
	_lora_module.startLoRaMesh();

	// log in sd card
	}

	{
	// clock interrupt wakes device
	// log in sd card
	}

	// go back to sleep
}