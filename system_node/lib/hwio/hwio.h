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

#define HWID_A      A0
#define HWID_B      A1
#define HWID_C      A2
#define BATT_IN     A3
#define SMOIS_IN    A7
#define SENS_TOGGLE 4
#define STEMP_IN    6
#define LORA_TOGGLE 7 

#define R1          10000
#define R2          33000
#define ADC_REF_VOL 3.3
#define ADC_RESO    1023

#define OFF         0
#define ON          1

#include "../system_node.hpp"

class HWIO_class
{
    private:
        uint8_t _hwid[HWID_PINS] = { HWID_A, HWID_B, HWID_C };
        Adafruit_AHTX0 _aht10;

        void setGPIO();
        void getHWID(uint8_t &hwid);
        void setAHT10();
        void getAHT10(float &temperature, float &humidity);
        void getBattery(float &battery);

    public:
        enum _commandList : uint8_t
        {
            NO_COMMAND,
            GPIO_SLEEP,
            GPIO_WAKE,
            LORA_SLEEP,
            LORA_WAKE
        };

        void Initialize(IDATA *IData);
        void Initialize_Modules(IDATA *IData);
        void loadSensorData(IDATA *IData);
        void toggleModules(uint8_t command);
        void toggleModules(uint8_t command1, uint8_t command2);
};

#endif