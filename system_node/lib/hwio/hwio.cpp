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

void HWIO_class::Initialize(IDATA *IData)
{
    noInterrupts();

    #ifdef DEBUGGING
        Serial.begin(SERIAL_BAUD);
    #endif

    setGPIO();
    getHWID(IData->HW_ID);

    interrupts();
}

void HWIO_class::Initialize_Modules(IDATA *IData)
{
    setAHT10();
}


void HWIO_class::loadSensorData(IDATA *IData)
{
    getAHT10(IData->SYSTEM_TEMPERATURE, IData->SYSTEM_HUMIDITY);
}

void HWIO_class::setGPIO()
{
    for (uint8_t i = 0; i < HWID_PINS; i++)
    {
        digitalWrite(_hwid[i], INPUT_PULLUP);
    }
}

void HWIO_class::getHWID(uint8_t &hwid)
{
    hwid = digitalRead(_hwid[0]);
    hwid += digitalRead(_hwid[1]) * 2;
    hwid += digitalRead(_hwid[2]) * 4;

    #ifdef DEBUGGING
        Serial.print("System Hardware ID: ");
        Serial.println(hwid);
    #endif
}

void HWIO_class::setAHT10()
{
    if (!_aht10.begin())
    {
        #ifdef DEBUGGING
            Serial.println("ERROR: Failed to Initialize AHT10!");
        #endif

        return;
    }

    #ifdef DEBUGGING
        Serial.println("AHT10 Setup Complete!");
    #endif
}

void HWIO_class::getAHT10(float &temperature, float &humidity)
{
    sensors_event_t aht10_humidity, aht10_temperature;
    
    if (_aht10.getEvent(&aht10_humidity, &aht10_temperature))
    {
        #ifdef DEBUGGING
            Serial.print("Temperature: ");
            Serial.print(aht10_temperature.temperature);
            Serial.println(" C");

            Serial.print("Humidity: ");
            Serial.print(aht10_humidity.relative_humidity);
            Serial.println("% rH");
        #endif

        temperature = aht10_temperature.temperature;
        humidity = aht10_humidity.relative_humidity;
    }
    else
    {
        #ifdef DEBUGGING
            Serial.println("ERROR: Failed to read AHT10 sensor data.");
        #endif
    }
}