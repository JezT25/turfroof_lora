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

// todo: figure out how long the whole lora process is like < 5 minutes? and then we can trim 2nd alarm.
// maybe also trim the timouets? or maybe not.... LORA_WAKE_TIMEOUT? idk seems ok

#include "lib/system_node.hpp"

SYSTEM_class system_node(class_lib);

void setup()
{
	system_node.Initialize();
}

void loop()
{
	system_node.Run();
}
