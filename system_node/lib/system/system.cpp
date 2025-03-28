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
volatile bool SYSTEM_class::_wokenbyLoRa = false;  

void SYSTEM_class::Initialize()
{
	// Initialize Hardware
	_hwio.Initialize(&_IData);

	// Intialize Modules
	_hwio.Initialize_Modules(&_IData);

	// Initialize Lora Module
	_lora_module.Initialize(_IData);

	#ifdef DEBUGGING
		Serial.print("System Version: v");
		Serial.println(SYSTEM_VER);
		Serial.println(".........................");
	#endif
}

void SYSTEM_class::Run()
{
	entersleepMode();

	_hwio.loadSensorData(&_IData);

	if (_wokenbyLoRa)
	{
		_lora_module.startLoRaMesh(_IData);
		_wokenbyLoRa = false;
	}

	// _sd_module.logdatatoSD();

	#ifdef DEBUGGING
		displayfreeRAM();
	#endif
}


void SYSTEM_class::entersleepMode()
{
    #ifdef DEBUGGING
        Serial.println("Entering Sleep Mode...");
        delay(50);
    #endif

	noInterrupts();

	// Set LoRa to Recieve mode to wake on signal
	// TODO: Set LoRa to lower Power?
	LoRa.receive();

	// Turn off other devices


	// Disable ADC and watchdog to save power
    ADCSRA = OFF;
    WDTCSR = OFF;

	// Attach interrupts for wake
	attachInterrupt(digitalPinToInterrupt(LORA_DI0), wakeonLoRa, RISING);
	attachInterrupt(digitalPinToInterrupt(RTC_INT), wakeonRTC, FALLING);
	EIFR = bit(INTF0);

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

	// Disable interrupts
	detachInterrupt(digitalPinToInterrupt(LORA_DI0));
	detachInterrupt(digitalPinToInterrupt(RTC_INT));

	#ifdef DEBUGGING
		delay(50);
		_wokenbyLoRa ? Serial.println("LoRa Woke Me Up!") : Serial.println("RTC Woke Me Up!");	
	#endif
}

inline void SYSTEM_class::wakeonLoRa()
{
	_wokenbyLoRa = true;
}

inline void SYSTEM_class::wakeonRTC()
{

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
		Serial.print(F("SRAM Left = "));
		Serial.println(freeRAM());
	}
#endif