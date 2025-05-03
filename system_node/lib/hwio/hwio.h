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

#define MAX_DEVICES		8
#define HWID_PINS   	3

#define HWID_A      	A3
#define HWID_B      	A2
#define HWID_C      	A1
#define BATT_IN     	A0
#define SMOIS_IN    	A7
#define SENS_TOGGLE 	4
#define LORA_TOGGLE 	5
#define STEMP_IN    	7

#define R1          	10000
#define R2          	33000
#define ADC_REF_VOL 	3.3
#define ADC_RESO_MIN	0
#define ADC_RESO_MAX   	1023
#define MIN_PERCENT		0
#define MAX_PERCENT 	100
#define MIN_VALUE		-99
#define MAX_VALUE		100

#define DATA_SAMPLES	10
#define SAMPLE_DELAY	50

#define OFF         	0
#define ON          	1

#include "../system_node.hpp"

class HWIO_class
{
	private:
		uint8_t _hwid[HWID_PINS] = { HWID_A, HWID_B, HWID_C };
		Adafruit_AHTX0 aht10;
		OneWire oneWire;
    	DallasTemperature ds18b20;

		void setGPIO();
		void getHWID(uint8_t &hwid);
		void getBattery(float &battery);
		void getAHT10(float &temperature, float &humidity);
		void getSoilTemperature(float &temperature);
		void getSoilMoisture(uint8_t &moisture);

	public:
		HWIO_class() : oneWire(STEMP_IN), ds18b20(&oneWire) {}

		enum _commandList : uint8_t
		{
			NO_COMMAND,
			GPIO_SLEEP,
			GPIO_WAKE,
			LORA_SLEEP,
			LORA_WAKE
		};

		void Initialize(IDATA *IData);
		void loadSensorData(IDATA *IData);
		void toggleModules(uint8_t command);
		void toggleModules(uint8_t command1, uint8_t command2);
};

#endif