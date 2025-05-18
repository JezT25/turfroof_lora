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

// Define static functions
volatile bool SYSTEM_class::_interruptbyLoRa = false;
volatile bool SYSTEM_class::_interruptbyRTC = false;  

void SYSTEM_class::Initialize()
{
	// Turn on modules first!
	_hwio.toggleModules(_hwio.GPIO_WAKE, _hwio.LORA_WAKE);

	// Initialize Hardware
	_hwio.Initialize(&_IData);

	// Initialize RTC Module
	_rtc_module.Initialize();

	// Initialize Lora Module
	_lora_module.Initialize(_IData);

	#ifdef DEBUGGING
		Serial.print(F("System Version: v"));
		Serial.println(SYSTEM_VER);
		Serial.println(F("........................."));
	#endif

	// Enter Sleep Mode
	entersleepMode();
}

void SYSTEM_class::Run()
{
	if (_interruptbyRTC)
	{
		// Wake RTC First!
		_hwio.toggleModules(_hwio.GPIO_WAKE);
		delay(DELAY_SMALL);
		_rtc_module.reInit();

		uint8_t alarm_trigger = _rtc_module.checkAlarm();
		_interruptbyRTC = false;

		if (alarm_trigger == ALARM1_TRIGGER)
		{
			_hwio.loadSensorData(&_IData);
			_sd_card_module.logData(_IData, _rtc_module.getTime());
			_lora_module.loadSensorData(_IData);
		}
		else if (alarm_trigger == ALARM2_TRIGGER)
		{
			#ifdef DEBUGGING
				displayfreeRAM();
			#endif

			entersleepMode();
		}
	}
	else if (_interruptbyLoRa)
	{
		_interruptbyLoRa = false;
		_lora_module.startLoRaMesh(_IData, &_hwio, &_rtc_module);
	}
	else
	{
		enterlightsleepMode();
	}

	wdt_reset();
}

void SYSTEM_class::entersleepMode()
{
	#ifdef DEBUGGING
		Serial.println(F("Entering Sleep Mode..."));
		delay(DELAY_SMALL);
	#endif

	// Turn off LoRa and Modules
	_hwio.toggleModules(_hwio.GPIO_SLEEP, _hwio.LORA_SLEEP);

	// Turn off communication to avoid power transmitted in GPIO
	SPI.end();
	Wire.end();
	_lora_module.setPinsOff();
	_hwio.setPinsOff();

	// Remove LoRa wake privileges and clear interrupt flags
	noInterrupts();
	detachInterrupt(digitalPinToInterrupt(LORA_DI0));
	attachInterrupt(digitalPinToInterrupt(RTC_INT), wakeonRTC, FALLING);
	EIFR = bit(INTF0);
	EIFR = bit(INTF1);
	gotosleep();

	// Turn on devices
	attachInterrupt(digitalPinToInterrupt(LORA_DI0), wakeonLoRa, RISING);
	_hwio.toggleModules(_hwio.LORA_WAKE);

	// Boot Devices
	delay(DELAY_SMALL);
	_lora_module.reInit();
}

void SYSTEM_class::enterlightsleepMode()
{
	#ifdef DEBUGGING
		Serial.println(F("Entering Light Sleep Mode..."));
		delay(DELAY_SMALL);
	#endif

	// Turn off other devices
	_hwio.toggleModules(_hwio.GPIO_SLEEP);
	LoRa.receive();	

	// Turn off communication to avoid power transmitted in GPIO
	Wire.end();
	_hwio.setPinsOff();

	// Set Interrupts and Sleep
	noInterrupts();
	attachInterrupt(digitalPinToInterrupt(LORA_DI0), wakeonLoRa, RISING);
	EIFR = bit(INTF0);
	gotosleep();

	// Remove LoRa Interrupt
	detachInterrupt(digitalPinToInterrupt(LORA_DI0));
}

inline void SYSTEM_class::gotosleep()
{
	// Disable WDT
	wdt_disable();

	// Disable ADC and watchdog to save power
	ADCSRA = OFF;
	WDTCSR = OFF;

	// Set sleep settings and interrupt wake
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	power_adc_disable();
	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();

	sleep_enable();

	// Disable BOD for lowest power
	MCUCR = bit (BODS) | bit (BODSE);
	MCUCR = bit (BODS);

	// Lower clock frequency
	clock_prescale_set(clock_div_256);

	// Enter sleep mode (Execution Stops Here)
	interrupts();
	sleep_cpu();

	////////////// zzz Goodnight! zzz //////////////

	// Wake up here!
	sleep_disable();

	// Power on!
	power_all_enable();
	ADCSRA = (ON << ADEN); 
	clock_prescale_set(clock_div_1);

	// Reenable WDT
	wdt_enable(WDTO_8S);
}

inline void SYSTEM_class::wakeonLoRa()
{
	_interruptbyLoRa = true;
}

inline void SYSTEM_class::wakeonRTC()
{
	_interruptbyRTC = true;
}

#ifdef DEBUGGING
	uint16_t SYSTEM_class::freeRAM() 
	{
		extern uint16_t __heap_start,*__brkval;
		uint16_t v;
		return (uint16_t)&v - (__brkval == 0 ? (uint16_t)&__heap_start : (uint16_t) __brkval);  
	}

	void SYSTEM_class::displayfreeRAM()
	{
		Serial.print(F("\033[31mSRAM Left = "));
		Serial.print(freeRAM());
		Serial.println(F("\033[0m"));
	}
#endif