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

#ifndef system_h
#define system_h

#include "../system_node.hpp"

class SYSTEM_class {
    private:
        IDATA&              _IData;
        ISYSTEM&            _ISystem;
        HWIO_class&         _hwio;
        LORA_MODULE_class&  _lora_module;
        // battery level?
        // DS3231_class
        // TEMP_HUMI_class
        // SOIL_MOISTURE
        // SD_CARD
        // encryption
        // maybe signal mesh checking?
        // sleep mode

        // logbook write and serial write

    public:
        SYSTEM_class(SystemComponents& class_lib) : _IData(class_lib._IData), _ISystem(class_lib._ISystem), _hwio(class_lib._hwio), _lora_module(class_lib._lora_module) {}
        
        void Initialize();
        void Run();
};

#endif
