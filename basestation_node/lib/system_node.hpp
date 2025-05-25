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

#define SERIAL_BAUD     115200

#define DEBUGGING
#define ENCRYPTING
// #define SYNC_ON								// Turn this on if we want to sync clocks after query
#ifdef ENCRYPTING
	#define RC4_BYTES 		255
	#define ENCRYPTION_KEY  "G7v!Xz@a?>Qp!d$1"
#endif

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <LoRa.h>

#include "config.hpp"
#include "IDevice.h"
#include "iot.h"
#include "iot.cpp"
#include "lora_module.h"
#include "lora_module.cpp"

struct SystemComponents
{
	IDATA      			_IData;

	IOT_class			_iot;
	LORA_MODULE_class   _lora_module;
};
SystemComponents class_lib;

#include "system.h"
#include "system.cpp"

#endif
