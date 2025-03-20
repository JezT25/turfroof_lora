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

#define SYSTEM_VER      0.04

#define DEBUG_ON

#define SERIAL_BAUD     115200
#define DECIMAL_VALUES  3

#include <Wire.h>
// #include <SPI.h>
#include <LoRa.h>
// #include <SD.h>
// #include <DS3231.h>
// #include <Adafruit_AHTX0.h>

#include "IDevice.h"
#include "hwio/hwio.h"
#include "hwio/hwio.cpp"
#include "lora_module/lora_module.h"
#include "lora_module/lora_module.cpp"

struct SystemComponents
{
    // Data Handling Classes
    IDATA               _IData;
    ISYSTEM             _ISystem;

    // Hardware Classes
    HWIO_class          _hwio;
    LORA_MODULE_class   _lora_module;
};
SystemComponents class_lib;

#include "system/system.h"
#include "system/system.cpp"

#endif
