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

#include "system_node.hpp"


void SYSTEM_class::Initialize()
{
    // Initialize HW Stuff
    Initialize_System();

    // Initialize IoT
    _iot.Initialize();

    // Initialize LoRa
    _lora_module.Initialize();
}

void SYSTEM_class::Run()
{
    // Keep looping get time until we are at HOUR:00:30 then start quereyinh
    // _iot.getTime();
    // delay(5000);

    // If we are at hour do lora stuff
    _lora_module.startLoRaMesh(_IData);

    // uplaod to cloud
    //_iot
}

void SYSTEM_class::Initialize_System()
{
    #ifdef DEBUGGING
        Serial.begin(SERIAL_BAUD);
    #endif
}