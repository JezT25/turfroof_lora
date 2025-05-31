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

void RTC_MODULE_class::Initialize()
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

	#if defined(DEBUGGING) && SET_RTC_TIME
		settimefromPC();
	#endif

	// Sync Clock with MCU
	Sync();

	// Set Alarm 1 to trigger every hour
	// _rtc.setAlarm(DS3232RTC::ALM1_MATCH_MINUTES, 0, ALARM1_MIN, 0, 1);
	_rtc.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 30, 0, 0, 1);		// TODO: EDIT ME
	_rtc.alarm(DS3232RTC::ALARM_1);
	_rtc.alarmInterrupt(DS3232RTC::ALARM_1, true);

	// Set Alarm 2 to trigger at ALARM2_MIN every hour
	_rtc.setAlarm(DS3232RTC::ALM2_MATCH_MINUTES, 0, ALARM2_MIN, 0, 1);
	_rtc.alarm(DS3232RTC::ALARM_2);
	_rtc.alarmInterrupt(DS3232RTC::ALARM_2, false);

	// Turn on BBSQW - Battery-Backed Square Wave Enable for battery interrupts
	byte reg = _rtc.readRTC(DS3232RTC::DS32_CONTROL);
	reg |= (ON << DS3232RTC::DS32_BBSQW);
	_rtc.writeRTC(DS3232RTC::DS32_CONTROL, reg);
}

inline void RTC_MODULE_class::reInit()
{
	_rtc.begin();
}

void RTC_MODULE_class::Sync()
{
	setSyncProvider(_rtc.get);
	#ifdef DEBUGGING
		if (timeStatus() != timeSet)
		{
			Serial.print(F("X: ERROR: Failed to Sync RTC! "));
		}
		Serial.print(F("RTC Sync: "));
		printtimedate(_rtc.get());
	#endif
}

uint8_t RTC_MODULE_class::checkAlarm()
{
	if (_rtc.alarm(DS3232RTC::ALARM_1))
	{
		#ifdef DEBUGGING
			Serial.print(F("Alarm 1 Triggered! "));
			printtimedate(_rtc.get());
		#endif

		return ALARM1_TRIGGER;
	}
	if (_rtc.alarm(DS3232RTC::ALARM_2))
	{
		#ifdef DEBUGGING
			Serial.print(F("Alarm 2 Triggered! "));
			printtimedate(_rtc.get());
		#endif

		return ALARM2_TRIGGER;
	}

	return NO_TRIGGER;
}

time_t RTC_MODULE_class::getTime()
{
	return _rtc.get();
}

void RTC_MODULE_class::syncTime(const float *rtctime)
{
	uint8_t hr = (uint8_t)rtctime[HOUR];
	uint8_t min = (uint8_t)rtctime[MINUTE];
	uint8_t sec = (uint8_t)rtctime[SECOND];
	uint8_t dy = (uint8_t)rtctime[DAY];
	uint8_t mnth = (uint8_t)rtctime[MONTH];
	uint8_t yr = (uint8_t)rtctime[YEAR];

	// Check data validity if it's in range
	if(hr > 23 || min > 59 || sec > 59 || dy < 1 || dy > 31 || mnth < 1 || mnth > 12 || yr > 99)
	{
		#ifdef DEBUGGING
			Serial.println(F("X: Invalid datettime Data Found!"));
		#endif

		return;
	}

	// Set the RTC module time
	setTime(hr, min, sec, dy, mnth, yr);
	_rtc.set(now());

	#ifdef DEBUGGING
		Serial.print(F("Datetime from LoRa: "));
		Serial.print(hr);Serial.print(':');
		Serial.print(min);Serial.print(':');
		Serial.print(sec);Serial.print(' ');
		Serial.print(dy);Serial.print('-');
		Serial.print(mnth);Serial.print('-');
		Serial.println(yr + 2000);
	#endif

	// Sync for good measure
	Sync();
}

#ifdef DEBUGGING
	void RTC_MODULE_class::settimefromPC()
	{
		Serial.println(F("Setting RTC"));

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
		Serial.print(hour(t));
		Serial.print(':');
		if(minute(t) < 10) Serial.print('0');
		Serial.print(minute(t));
		Serial.print(':');
		if(second(t) < 10) Serial.print('0');
		Serial.print(second(t));
		Serial.print(' ');
		Serial.print(day(t));
		Serial.print('-');
		Serial.print(month(t));
		Serial.print('-');
		Serial.println(year(t));
	}
#endif
