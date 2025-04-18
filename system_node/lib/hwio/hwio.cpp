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

	// Begin Serial Connection
	Serial.begin(SERIAL_BAUD);

	// Get HW ID
	setGPIO();
	getHWID(IData->HW_ID);

	interrupts();
}

void HWIO_class::loadSensorData(IDATA *IData)
{
	getAHT10(IData->SYSTEM_TEMPERATURE, IData->SYSTEM_HUMIDITY);
	getSoilTemperature(IData->SOIL_TEMPERATURE);
	getSoilMoisture(IData->SOIL_MOISTURE);
	getBattery(IData->BATTERY_VOLTAGE);
}

void HWIO_class::setGPIO()
{
	for (uint8_t i = 0; i < HWID_PINS; i++)
	{
		pinMode(_hwid[i], INPUT_PULLUP);
	}

	pinMode(SENS_TOGGLE, OUTPUT);
	pinMode(LORA_TOGGLE, OUTPUT);
}

void HWIO_class::getHWID(uint8_t &hwid)
{
	hwid = digitalRead(_hwid[0]);
	hwid += digitalRead(_hwid[1]) * 2;
	hwid += digitalRead(_hwid[2]) * 4;

	#ifdef DEBUGGING
		Serial.print(F("System Hardware ID: "));
		Serial.println(hwid);
	#endif

	for (uint8_t i = 0; i < HWID_PINS; i++)
	{
		pinMode(_hwid[i], INPUT);
	}
}

void HWIO_class::toggleModules(uint8_t command)
{
	toggleModules(command, NO_COMMAND);
}

void HWIO_class::toggleModules(uint8_t command1, uint8_t command2)
{
	if (command1 == GPIO_SLEEP || command2 == GPIO_SLEEP)
	{
		digitalWrite(SENS_TOGGLE, OFF);
	}
	if (command1 == GPIO_WAKE || command2 == GPIO_WAKE)
	{
		digitalWrite(SENS_TOGGLE, ON);
	}
	if (command1 == LORA_SLEEP || command2 == LORA_SLEEP)
	{
		digitalWrite(LORA_TOGGLE, OFF);
	}
	if (command1 == LORA_WAKE || command2 == LORA_WAKE)
	{
		digitalWrite(LORA_TOGGLE, ON);
	}
}

void HWIO_class::getBattery(float &battery)
{
	float totaldatasamples = 0;
	for (uint8_t i = 0; i < DATA_SAMPLES; i++)
	{
		totaldatasamples += analogRead(BATT_IN);
		delay(SAMPLE_DELAY);
	}

	float pinvoltage = ((totaldatasamples / DATA_SAMPLES) / ADC_RESO_MAX) * ADC_REF_VOL;
	float batteryvoltage = pinvoltage * ((R1 + R2) / R2);

	#ifdef DEBUGGING
		Serial.print(F("Battery Voltage: "));
		Serial.print(batteryvoltage);
		Serial.println(F(" Volts"));
	#endif

	battery = constrain(batteryvoltage, MIN_VALUE, MAX_VALUE);
}

void HWIO_class::getAHT10(float &temperature, float &humidity)
{
	sensors_event_t aht10_humidity, aht10_temperature;

	// Initialize AHT10
	aht10.begin();

	if (aht10.getEvent(&aht10_humidity, &aht10_temperature))
	{
		#ifdef DEBUGGING
			Serial.print(F("Temperature: "));
			Serial.print(aht10_temperature.temperature);
			Serial.println(F(" C"));

			Serial.print(F("Humidity: "));
			Serial.print(aht10_humidity.relative_humidity);
			Serial.println(F("% rH"));
		#endif

		temperature = constrain(aht10_temperature.temperature, MIN_VALUE, MAX_VALUE);
		humidity = constrain(aht10_humidity.relative_humidity, MIN_VALUE, MAX_VALUE);
	}
	else
	{
		#ifdef DEBUGGING
			Serial.println(F("X: ERROR: Failed to read AHT10 sensor data."));
		#endif
	}
}

void HWIO_class::getSoilTemperature(float &temperature)
{
	// Initialize DS18B20
	ds18b20.begin();
	ds18b20.requestTemperatures();

	float soil_temperature = ds18b20.getTempCByIndex(0); 

	#ifdef DEBUGGING
		Serial.print(F("Soil Temperature: "));
		Serial.print(soil_temperature);
		Serial.println(F(" C"));
	#endif

	temperature = constrain(soil_temperature, MIN_VALUE, MAX_VALUE);
}

void HWIO_class::getSoilMoisture(uint8_t &moisture)
{
	int totaldatasamples = 0;
	for (uint8_t i = 0; i < DATA_SAMPLES; i++)
	{
		totaldatasamples += analogRead(SMOIS_IN);
		delay(SAMPLE_DELAY);
	}

	uint8_t moisturepercentage = map((analogRead(SMOIS_IN) / DATA_SAMPLES), ADC_RESO_MIN, ADC_RESO_MAX, MIN_PERCENT, MAX_PERCENT);

	#ifdef DEBUGGING
		Serial.print(F("Soil Moisture: "));
		Serial.print(moisturepercentage);
		Serial.println(F(" %"));
	#endif

	moisture = constrain(moisturepercentage, MIN_VALUE, MAX_VALUE);
}