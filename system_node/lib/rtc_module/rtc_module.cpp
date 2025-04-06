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

bool RTC_MODULE_class::Initialize()
{
	// Initialize and Clear Flags
    _rtc.begin();
	_rtc.setAlarm(DS3232RTC::ALM1_MATCH_DATE, 0, 0, 0, 1);
    _rtc.setAlarm(DS3232RTC::ALM2_MATCH_DATE, 0, 0, 0, 1);
    _rtc.alarm(DS3232RTC::ALARM_1);
    _rtc.alarm(DS3232RTC::ALARM_2);
    _rtc.alarmInterrupt(DS3232RTC::ALARM_1, false);
    _rtc.alarmInterrupt(DS3232RTC::ALARM_2, false);
    _rtc.squareWave(DS3232RTC::SQWAVE_NONE);

	#ifdef SET_RTC_TIME
		settimefromPC();
	#endif

	// Sync Clock with MCU
    Sync();

    // TODO: EDIT ME | Set Alarm1 every hour to wake up
    _rtc.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 30, 0, 0, 1);
    _rtc.alarm(DS3232RTC::ALARM_1);
    _rtc.alarmInterrupt(DS3232RTC::ALARM_1, true);

    // TODO: EDIT ME | Set Alarm2 after 5 minutes to go back to sleep
    _rtc.setAlarm(DS3232RTC::ALM2_EVERY_MINUTE, 0, 0, 0, 1);
    _rtc.alarm(DS3232RTC::ALARM_2);
    _rtc.alarmInterrupt(DS3232RTC::ALARM_2, true);

	// Turn on BBSQW - Battery-Backed Square Wave Enable for battery interrupts
    byte reg = _rtc.readRTC(DS3232RTC::DS32_CONTROL);
    reg |= (ON << DS3232RTC::DS32_BBSQW);
    _rtc.writeRTC(DS3232RTC::DS32_CONTROL, reg);
}

bool RTC_MODULE_class::Sync()
{
	setSyncProvider(_rtc.get);
	#ifdef DEBUGGING
		if (timeStatus() != timeSet)
		{
            Serial.println("ERROR: Failed to Sync RTC!");
		}
		printtimedate(_rtc.get());
    #endif
}

uint8_t RTC_MODULE_class::checkAlarm()
{
	if (_rtc.alarm(DS3232RTC::ALARM_1))
	{
		#ifdef DEBUGGING
			Serial.print("Alarm 1 Triggered! ");
			printtimedate(_rtc.get());
		#endif

		return ALARM1_TRIGGER;
	}
	if (_rtc.alarm(DS3232RTC::ALARM_2))
	{
		#ifdef DEBUGGING
			Serial.print("Alarm 2 Triggered! ");
			printtimedate(_rtc.get());
		#endif

		return ALARM2_TRIGGER;
	}

	return NO_TRIGGER;
}

#ifdef DEBUGGING
	void RTC_MODULE_class::settimefromPC()
	{
		Serial.println("Setting RTC");

		// Extract compile-time and compile-date
		char mth[4];
		uint8_t hour, minute, second, year, month, day;
		sscanf(__TIME__, "%hhu:%hhu:%hhu", &hour, &minute, &second);
		sscanf(__DATE__, "%s %hhu %hhu", mth, &day, &year);

		// Convert month name to number
		const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		for (month = 0; month < 12; month++)
		{
			if (strncmp(mth, months[month], 3) == 0) break;
		}
		month++;  // Convert from 0-based index to 1-based month

		// Convert year to DS3232RTC format (offset from 2000)
		year -= 2000;

		// Set the time using the DS3232RTC library
		setTime(hour, minute, second, day, month, year);
		_rtc.set(now());  // Set the RTC module time
	}

	void RTC_MODULE_class::printtimedate(time_t t)
	{
		Serial.print("RTC Time: ");
		Serial.print(hour());
		Serial.print(':');
		if(minute() < 10) Serial.print('0');
		Serial.print(minute());
		Serial.print(':');
		if(second() < 10) Serial.print('0');
		Serial.print(second());
		Serial.print(' ');
		Serial.print(day());
		Serial.print('-');
		Serial.print(month());
		Serial.print('-');
		Serial.println(year());
	}
#endif
