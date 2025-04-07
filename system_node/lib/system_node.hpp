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

#define SYSTEM_VER      0.06

// Toggles: Comment or Uncomment to Disable or Enable
#define DEBUGGING
// #define ENCRYPTING
// #define SET_RTC_TIME

// Debugging Settings
#define SERIAL_BAUD     115200

// Encryption Settings
// TODO: Change up encryption when Serial is Off
#define RC4_BYTES			  128
#define ENCRYPTION_KEY  "G7v!Xz@pL9#qKd2M"

#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <SDCore.h>
#include <OneWire.h>
#include <DS3232RTC.h>
#include <DallasTemperature.h>
#include <Adafruit_AHTX0.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include "IDevice.h"
#include "hwio/hwio.h"
#include "hwio/hwio.cpp"
#include "rtc_module/rtc_module.h"
#include "rtc_module/rtc_module.cpp"
#include "lora_module/lora_module.h"
#include "lora_module/lora_module.cpp"

struct SystemComponents
{
    // Data Handling Classes
    IDATA               _IData;

    // Hardware Classes
    HWIO_class          _hwio;
    RTC_MODULE_class    _rtc_module;
    LORA_MODULE_class   _lora_module;
};
SystemComponents class_lib;

#include "system/system.h"
#include "system/system.cpp"

#endif
