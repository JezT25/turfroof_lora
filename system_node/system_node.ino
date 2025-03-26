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

	// TODO: BUY
	// soldering lead!
	// battery 18650!
	// SD CARD MODULE 3.3V
	// LORA broken and alternatives
	// dip switches
	// rtc?

	// DONE BUY:
	// CHANGE TO PRO MINI 3.3V
	// buy ds18b20 temp sensor
	// soil moisture sensor

	// make sure to power in proper pin to reduce current
	// MAKE SURE TO POWER SENSORS USING GPIO PINS FOR SAVING POWER!!!!!!!!
	// CAN LORA SLEEP?

	// check how many bounces and determine somethingd wrong
	// magic bytes?
	// for thesis graph the backout and timeout

	//Method:
	// sleep all > alarm 1 > wake-turn on lora - sleep mcu > lora things > alarm2 > sd card > sleep all
	

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
