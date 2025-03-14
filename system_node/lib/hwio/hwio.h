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

#ifndef hwio_h
#define hwio_h

#define MAX_DEVICES 8

#define HWID_PINS   3
#define HWID_A      14
#define HWID_B      15
#define HWID_C      16

#include "../system_node.hpp"

class HWIO_class {
    private:
        uint8_t _hwid[HWID_PINS] = { HWID_A, HWID_B, HWID_C };

        void setGPIO();
        void getHWID(ISYSTEM *ISystem);

    public:
        void Initialize(ISYSTEM *ISystem);
};

#endif
