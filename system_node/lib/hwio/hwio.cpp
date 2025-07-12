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
		// Begin Serial Connection for Debugging
		Serial.begin(SERIAL_BAUD);
	#endif

	// Get HW ID
	setGPIO();
	getHWID(IData->HW_ID);

	interrupts();

	toggleModules(GPIO_WAKE, LORA_WAKE);
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

void HWIO_class::setPinsOff()
{
	#ifndef DEBUGGING
		// Set Serial Off
		pinMode(PIN_TX, INPUT);
		pinMode(PIN_RX, INPUT);
	#endif

	// Set I2C Off
	pinMode(PIN_SDA, INPUT);
	pinMode(PIN_SCL, INPUT);
}

void HWIO_class::getHWID(uint8_t &hwid)
{
	hwid = !digitalRead(_hwid[0]);
	hwid += !digitalRead(_hwid[1]) * 2;
	hwid += !digitalRead(_hwid[2]) * 4;

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

	// Purge initial boot up readings
	for (uint8_t i = 0; i < DATA_SAMPLES; i++)
	{
		analogRead(BATT_IN);
		delay(SAMPLE_DELAY);
	}

	// Actual Readings
	for (uint8_t i = 0; i < DATA_SAMPLES; i++)
	{
		totaldatasamples += analogRead(BATT_IN);
		delay(SAMPLE_DELAY);
	}

	float pinvoltage = ((totaldatasamples / DATA_SAMPLES) / ADC_RESO_MAX) * ADC_REF_VOL;
	float batteryvoltage = pinvoltage * (((float)R1 + (float)R2) / (float)R2);

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

	delay(LONG_DELAY);

	float soil_temperature = ds18b20.getTempCByIndex(0); 

	#ifdef DEBUGGING
		Serial.print(F("Soil Temperature: "));
		Serial.print(soil_temperature);
		Serial.println(F(" C"));
	#endif

	temperature = constrain(soil_temperature, MIN_VALUE, MAX_VALUE);
}

void HWIO_class::getSoilMoisture(uint16_t &moisture)
{
	uint16_t totaldatasamples = 0;

	// Purge initial boot up readings
	for (uint8_t i = 0; i < DATA_SAMPLES; i++)
	{
		analogRead(SMOIS_IN);
		delay(SAMPLE_DELAY);
	}

	// Actual Readings
	for (uint8_t i = 0; i < DATA_SAMPLES; i++)
	{
		totaldatasamples += analogRead(SMOIS_IN);
		delay(SAMPLE_DELAY);
	}

	uint16_t avgReading = totaldatasamples / DATA_SAMPLES;

	#ifdef DEBUGGING
		Serial.print(F("Soil Analog Read: "));
		Serial.println(avgReading);
	#endif

	moisture = avgReading;
}