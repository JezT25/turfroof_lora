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
	_hwio.Initialize(&_ISystem);

	// Initialize Lora Module
	_lora_module.Initialize(_ISystem);

	#ifdef DEBUGGING
		Serial.print("System Version: v");
		Serial.println(SYSTEM_VER);
		Serial.println(".........................");
	#endif
}

void SYSTEM_class::Run()
{
	// TODO: BUY
	// CHANGE TO PRO MINI 3.3V
	// buy ds18b20 temp sensor
	// sht31 for humidity outside
	// soil moisture sensor
	// battery 3.3!

	// MAKE SURE TO POWER SENSORS USING GPIO PINS FOR SAVING POWER!!!!!!!!
	// CAN LORA SLEEP?

	// make sure to power in proper pin to reduce current

	/*
		Summary of methods

		Use as many of the techniques listed here as are practical in your application. They are described in further detail below.


		Run the processor at a lower frequency
		Run the processor at a lower voltage
		Turn off unneeded internal modules in software (eg. SPI, I2C, Serial, ADC)
		Turn off brownout detection
		Turn off the Analog-to-Digital converter (ADC)
		Turn off the watchdog timer
		Put the processor to sleep
		Don't use inefficient voltage regulators - if possible run directly from batteries
		Don't use power-hungry displays (eg. indicator LEDs, backlit LCDs)
		Arrange to wake the processor from sleep only when needed
		Turn off (with a MOSFET) external devices (eg. SD cards, temperature sensors) until needed
	*/

	////////////////////////////////////////////////////////////////////////


	entersleepMode();

	// get sensor data

	if (_wokenbyLoRa)
	{
		_lora_module.loadSensorData(_IData);
		_lora_module.startLoRaMesh();
		_wokenbyLoRa = false;
	}

	// log in sd card
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
	// attachInterrupt(digitalPinToInterrupt(xxxxxxxxx), wakeonRTC, RISING);
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

	////////////// zzz Goodnight zzz //////////////

	// Wake up here!
	sleep_disable();

	// Power on!
	power_all_enable();
    ADCSRA = (ON << ADEN); 
	clock_prescale_set(clock_div_1);

	// Disable interrupts
	detachInterrupt(digitalPinToInterrupt(LORA_DI0));
	// detachInterrupt(digitalPinToInterrupt(xxxxxxxxx));

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