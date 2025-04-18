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

#ifndef system_node_hpp_included
#define system_node_hpp_included

#define SYSTEM_VER      0.71
#define SERIAL_BAUD     115200

// Toggles: Comment or Uncomment to Disable or Enable
#define DEBUGGING
// #define ENCRYPTING

// Encryption Settings
#ifdef DEBUGGING
	#define SET_RTC_TIME	0			// Set to 1 to set RTC time from PC
#endif

#ifdef ENCRYPTING
	#define RC4_BYTES		32
	#define ENCRYPTION_KEY  "G7v!X"
#endif

#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DS3232RTC.h>
#include <DallasTemperature.h>
#include <Adafruit_AHTX0.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "IDevice.h"
#include "hwio/hwio.h"
#include "hwio/hwio.cpp"
#include "rtc_module/rtc_module.h"
#include "rtc_module/rtc_module.cpp"
#include "lora_module/lora_module.h"
#include "lora_module/lora_module.cpp"
#include "sd_card_module/sd_card_module.h"
#include "sd_card_module/sd_card_module.cpp"

struct SystemComponents
{
	// Data Handling Classes
	IDATA                 _IData;

	// Hardware Classes
	HWIO_class            _hwio;
	RTC_MODULE_class      _rtc_module;
	LORA_MODULE_class     _lora_module;
	SD_CARD_MODULE_class  _sd_card_module;
};
SystemComponents class_lib;

#include "system/system.h"
#include "system/system.cpp"

#endif
