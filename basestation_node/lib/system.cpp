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

#include "system_node.hpp"


void SYSTEM_class::Initialize()
{
    // Initialize HW Stuff
    Initialize_System();

    // Initialize IoT
    _iot.Initialize();

    // Initialize LoRa
    _lora_module.Initialize();
}

void SYSTEM_class::Run()
{
    // Wait until the scheduled query time
    _iot.waitUntilQueryTime();

    // Clear previous sensor data
    clearData();

    // Collect sensor data via the LoRa mesh network
    _lora_module.startLoRaMesh(&_IData, &_iot);

    // Display collected data if debugging is enabled
    #ifdef DEBUGGING
        displayData();
    #endif

    // Upload the collected data to the cloud
    _iot.uploadData(_IData);
}

void SYSTEM_class::clearData()
{
    memset(_IData.TEMP_DATA, 0, sizeof(_IData.TEMP_DATA));
    memset(_IData.HUMI_DATA, 0, sizeof(_IData.HUMI_DATA));
    memset(_IData.STMP_DATA, 0, sizeof(_IData.STMP_DATA));
    memset(_IData.SMOI_DATA, 0, sizeof(_IData.SMOI_DATA));
    memset(_IData.BATT_DATA, 0, sizeof(_IData.BATT_DATA));
}

void SYSTEM_class::Initialize_System()
{
    #ifdef DEBUGGING
        Serial.begin(SERIAL_BAUD);
    #endif
}

#ifdef DEBUGGING
    void SYSTEM_class::displayData()
    {
        Serial.println("\n---- Sensor Data Contents ----");

        Serial.print("Air Temperature: [");
        for (uint8_t i = 0; i < MAX_DEVICES; ++i)
        {
            if (_IData.TEMP_DATA[i] == 0)
            {
                Serial.print(BLANK_PLACEHOLDER);
            }
            else
            {
                Serial.print(_IData.TEMP_DATA[i], DECIMAL_VALUES);
            }
            if (i < MAX_DEVICES - 1) Serial.print(", ");
        }
        Serial.println("]");

        Serial.print("Humidity: [");
        for (uint8_t i = 0; i < MAX_DEVICES; ++i)
        {
            if (_IData.HUMI_DATA[i] == 0)
            {
                Serial.print(BLANK_PLACEHOLDER);
            }
            else
            {
                Serial.print(_IData.HUMI_DATA[i], DECIMAL_VALUES);
            }
            if (i < MAX_DEVICES - 1) Serial.print(", ");
        }
        Serial.println("]");

        Serial.print("Soil Temperature: [");
        for (uint8_t i = 0; i < MAX_DEVICES; ++i)
        {
            if (_IData.STMP_DATA[i] == 0)
            {
                Serial.print(BLANK_PLACEHOLDER);
            }
            else
            {
                Serial.print(_IData.STMP_DATA[i], DECIMAL_VALUES);
            }
            if (i < MAX_DEVICES - 1) Serial.print(", ");
        }
        Serial.println("]");

        Serial.print("Soil Moisture: [");
        for (uint8_t i = 0; i < MAX_DEVICES; ++i)
        {
            if (_IData.SMOI_DATA[i] == 0)
            {
                Serial.print(BLANK_PLACEHOLDER);
            }
            else
            {
                Serial.print(_IData.SMOI_DATA[i]);
            }
            if (i < MAX_DEVICES - 1) Serial.print(", ");
        }
        Serial.println("]");

        Serial.print("Battery Voltage: [");
        for (uint8_t i = 0; i < MAX_DEVICES; ++i)
        {
            if (_IData.BATT_DATA[i] == 0)
            {
                Serial.print(BLANK_PLACEHOLDER);
            }
            else
            {
                Serial.print(_IData.BATT_DATA[i], DECIMAL_VALUES);
            }
            if (i < MAX_DEVICES - 1) Serial.print(", ");
        }
        Serial.println("]");

        Serial.println("----------------------------------------\n");
    }
#endif
