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

// make sure to power in proper pin to reduce current
// if battery lower than x amount forecver sleep and sd card only?

// check how many bounces and determine somethingd wrong
// for thesis graph the backout and timeout

//todo:
// CHECK FOR MEMORY LEAKS
// check input pullups?
// check if all are inputs?

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
