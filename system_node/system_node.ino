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
	// MAKE SURE TO POWER SENSORS USING GPIO PINS FOR SAVING POWER!!!!!!!!
	// CAN LORA SLEEP?
	// if battery lower than x amount forecver sleep and sd card only?

	// check how many bounces and determine somethingd wrong
	// magic bytes?
	// for thesis graph the backout and timeout

	//todo:
	// CHECK FOR MEMORY LEAKS
	// we might have to use WDT
	// check input pullups?
	// check if all are inputs?
	

	/*
		Summary of methods

		Use as many of the techniques listed here as are practical in your application. They are described in further detail below.


		Run the processor at a lower frequency
		Run the processor at a lower voltage
		Turn off unneeded internal modules in software (eg. SPI, I2C, Serial, ADC)
		Turn off brownout detection
		Turn off the Analog-to-Digital converter (ADC)
		Turn off the watchdog timer
		Put the processor to sleep
		Don't use inefficient voltage regulators - if possible run directly from batteries
		Don't use power-hungry displays (eg. indicator LEDs, backlit LCDs)
		Arrange to wake the processor from sleep only when needed
		Turn off (with a MOSFET) external devices (eg. SD cards, temperature sensors) until needed
	*/

	////////////////////////////////////////////////////////////////////////

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
