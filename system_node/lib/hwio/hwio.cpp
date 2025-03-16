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

#include "../system_node.hpp"

void HWIO_class::Initialize(ISYSTEM *ISystem)
{
    noInterrupts();
    Serial.begin(SERIAL_BAUD);
    setGPIO();
    getHWID(ISystem);
    interrupts();
}

void HWIO_class::setGPIO()
{
    for(int i = 0; i < HWID_PINS; i++)
    {
        digitalWrite(_hwid[i], INPUT_PULLUP);
    }
}

void HWIO_class::getHWID(ISYSTEM *ISystem)
{
    ISystem->HW_ID = digitalRead(_hwid[0]);
    ISystem->HW_ID += digitalRead(_hwid[1]) * 2;
    ISystem->HW_ID += digitalRead(_hwid[2]) * 4;

    Serial.print("Hardware ID: ");
    Serial.println(ISystem->HW_ID);
}
