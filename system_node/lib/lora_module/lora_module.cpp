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

void LORA_MODULE_class::Initialize(IDATA IData)
{	
	// Configure Pins
	LoRa.setPins(LORA_NSS, LORA_RST, LORA_DI0);

	// Start LoRa
	if (!LoRa.begin(FREQUENCY))
	{
		#ifdef DEBUGGING
			Serial.println(F("X: ERROR: LoRa Initialization Failed!"));
		#endif
	}

	// Setup Settings
	LoRa.setTxPower(TX_POWER);
	LoRa.setSignalBandwidth(BANDWIDTH);
	LoRa.setSyncWord(SYNC_WORD);
	LoRa.setSpreadingFactor(SPREAD_FACTOR);
	LoRa.setCodingRate4(CODING_RATE);
	LoRa.setPreambleLength(PREAMBLE);

	// Enable CRC
	LoRa.enableCrc();

	// Set HW ID
	_hwid = IData.HW_ID;

	// CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance) Parameters
	_backoffTime = _hwid * BACKOFF_MUL;
	_csmaTimeout = CSMA_TOUT_MIN + (CSMA_TOUT_MUL * _hwid);

	#ifdef DEBUGGING
		Serial.print(F("Backoff Time (ms): "));
		Serial.println(_backoffTime);
		Serial.print(F("CSMA Timeout (ms): "));
		Serial.println(_csmaTimeout);
		Serial.println(F("LoRa Setup Complete!"));
	#endif
}

void LORA_MODULE_class::loadSensorData(IDATA IData)
{
	_sensorData[TEMPERATURE] = IData.SYSTEM_TEMPERATURE;
	_sensorData[HUMIDITY] = IData.SYSTEM_HUMIDITY;
	_sensorData[SOIL_TEMPERATURE] = IData.SOIL_TEMPERATURE;
	_sensorData[SOIL_MOISTURE] = IData.SOIL_MOISTURE;
	_sensorData[BATT_VOLTAGE] = IData.BATTERY_VOLTAGE;

	resetValues();
}

void LORA_MODULE_class::startLoRaMesh(IDATA IData, RTC_MODULE_class rtc)
{
	while (millis() - _lastSystemUpdateTime <= LORA_WAKE_TIMEOUT)
	{
		if (LoRa.parsePacket() || _newpayloadAlert)
		{
			if (!_newpayloadAlert && !getLoRaPayload()) continue;
			preloadMessageData();
			processPayloadData();
			sendPayloadData(rtc);
		}

		wdt_reset();
	}
}

void LORA_MODULE_class::resetValues()
{
	// Reset values for fresh requests
	_sendAttempts = 0;
	_newpayloadAlert = false;	
	_lastSystemUpdateTime = millis();
	memset(_loraPayload, 0, sizeof(_loraPayload));
	memset(_loraprevHeader, 0, sizeof(_loraprevHeader));
	memset(_systemValues, 0, sizeof(_systemValues));
}

bool LORA_MODULE_class::getLoRaPayload()
{
	#ifdef DEBUGGING
		//	Display Signal Strength (RSSI) of the Received Packet
		Serial.println(F("Packet received!"));
		Serial.print(F("Signal strength [RSSI] (dBm): "));
		Serial.println(LoRa.packetRssi());
	#endif

	//	Get Payload content
	uint8_t payloadIndex = 0;
	memset(_loraPayload, 0, sizeof(_loraPayload));
	while (LoRa.available() && payloadIndex < sizeof(_loraPayload) - 1)
	{
		_loraPayload[payloadIndex++] = (char)LoRa.read();
	}
	_loraPayload[payloadIndex] = '\0';

	// Flush remaining bytes (if any)
	while (LoRa.available()) LoRa.read();

	#ifdef ENCRYPTING
		#ifdef DEBUGGING
			Serial.print(F("Encrypted Payload: "));
			Serial.println(_loraPayload);
		#endif

		char decryptedPayload[MAX_MESSAGE_LENGTH];
		strncpy(decryptedPayload, _loraPayload, sizeof(decryptedPayload));
		decryptedPayload[sizeof(decryptedPayload) - 1] = '\0';
		rc4EncryptDecrypt(decryptedPayload, strlen(decryptedPayload) + 1);
		strncpy(_loraPayload, decryptedPayload, sizeof(_loraPayload));
		_loraPayload[sizeof(_loraPayload) - 1] = '\0';
	#endif

	#ifdef DEBUGGING
		//	Print LoRa Payload
		Serial.print(F("Payload: "));
		Serial.println(_loraPayload);
	#endif

	return checkMessageValidity();
}

int8_t LORA_MODULE_class::getcharIndex(char c)
{
	for (uint8_t i = 0; i < sizeof(_loraPayload); i++)
	{
		if (_loraPayload[i] == c) return i;
	}
	return -1;
}

bool LORA_MODULE_class::checkMessageValidity()
{
	uint8_t startIdx = getcharIndex('[');
	uint8_t endIdx = getcharIndex(']');
	uint8_t payloadLen = strlen(_loraPayload);
	bool checkforCharacters = true;

	// Check header validity
	for (uint8_t i = 0; i < VALID_HEADERS; i++)
	{
		if (strncmp(_loraPayload, _validHeaders[i], strlen(_validHeaders[i])) == 0) break;

		if(i == VALID_HEADERS - 1)
		{
			#ifdef DEBUGGING
				Serial.println(F("X: Invalid Packet Header!"));
			#endif

			return false;
		}
	}

	// Check Payload data brackets
	if (startIdx == -1 && endIdx == -1)
	{
		#ifdef DEBUGGING
			Serial.println(F("X: Unable to Locate Data!"));
		#endif

		return false;
	}

	// Check Payload data strictly
	for (size_t i = START_OF_BRACKET; i < payloadLen; i++)
	{
		char payloadChar = _loraPayload[i];

		if (checkforCharacters)
		{
			if (payloadChar == BLANK_PLACEHOLDER || (payloadChar == '[' && _loraPayload[i + 1] == ']')) continue;
			
			if (payloadChar != '[' && payloadChar != ']' && payloadChar != ',')
			{
				#ifdef DEBUGGING
					Serial.println(F("X: Invalid Character Found!"));
				#endif

				return false;
			}

			if ((payloadChar == '[' && _loraPayload[i + 2] == ',') || (payloadChar == ',' && _loraPayload[i + 2] == ',') || (payloadChar == ',' && _loraPayload[i + 2] == ']'))
			{
				if (_loraPayload[i + 1] != BLANK_PLACEHOLDER)
				{
					#ifdef DEBUGGING
						Serial.println(F("X: Invalid Data Found!"));
					#endif

					return false;
				}
			}
			else
			{
				checkforCharacters = false;
			}
		}
		else
		{
			if ((payloadChar < '0' || payloadChar > '9') && payloadChar != '.' && payloadChar != '-')
			{
				#ifdef DEBUGGING
					Serial.println(F("Invalid Number Found!"));
				#endif

				return false;
			}

			if (_loraPayload[i + 1] == ',' || _loraPayload[i + 1] == ']')
			{
				checkforCharacters = true;
			}
		}
	}

	return true;
}

void LORA_MODULE_class::preloadMessageData()
{
	uint8_t startIdx = getcharIndex('[');
	uint8_t endIdx = getcharIndex(']');
	uint8_t payloadLen = strlen(_loraPayload);

	if ((startIdx == START_OF_BRACKET && endIdx == START_OF_BRACKET + 1 && payloadLen == START_OF_BRACKET + 2) || strncmp(_loraPayload, _loraprevHeader, START_OF_BRACKET) != 0)
	{
		// Reset database
		memset(_systemValues, 0, sizeof(_systemValues));

		// Load values
		for (uint8_t i = 0; i < VALID_HEADERS - 1; i++)
		{
			if (strncmp(_loraPayload, _validHeaders[i], strlen(_validHeaders[i])) == 0)
			{
				_systemValues[_hwid] = _sensorData[i];
				break;
			}
		}

		// Get new header
		strncpy(_loraprevHeader, _loraPayload, START_OF_BRACKET);
		_loraprevHeader[MAX_HEADER_LENGTH - 1] = '\0';
	}

	#ifdef DEBUGGING
		//	Print Current Database
		Serial.print(F("Current Data: ["));
		for (uint8_t i = 0; i < MAX_DEVICES; i++)
		{
			Serial.print(_systemValues[i], DECIMAL_VALUES);
			if (i < (MAX_DEVICES - 1)) Serial.print(',');
		}
		Serial.println(']');
	#endif
}

void LORA_MODULE_class::processPayloadData()
{
	// Extract data from payload
	uint8_t startIdx = getcharIndex('[');
	uint8_t endIdx = getcharIndex(']');
	uint8_t array_len = endIdx - (startIdx + 1);
	char buffer[array_len + 1];
	strncpy(buffer, &_loraPayload[startIdx + 1], array_len);
	buffer[array_len] = '\0';
	char *token = strtok(buffer, ",");

	// Store payload data to tempValues
	float tempValues[MAX_DEVICES] = {};
	uint8_t index = 0;
	while (token != nullptr && index < MAX_DEVICES)
	{
		tempValues[index] = atof(token);
		index++;
		token = strtok(nullptr, ",");
	}

	// Compare stored data with new data and determine if we need to resend
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		//	Merge current values with data from Payload only when new data is different
		if (fabs(tempValues[i] - _systemValues[i]) > EPSILON)
		{
			_sendAttempts = 0;

			for (uint8_t i = 0; i < MAX_DEVICES; i++)
			{
				if (i == _hwid) continue;
				_systemValues[i] = _systemValues[i] == 0 ? (tempValues[i] == 0 ? 0 : tempValues[i]) : _systemValues[i];
			}

			#ifdef DEBUGGING
				//	Print merged values
				Serial.print(F("Merged Data: ["));
				for (uint8_t i = 0; i < MAX_DEVICES; i++)
				{
					Serial.print(_systemValues[i], DECIMAL_VALUES);
					if (i < (MAX_DEVICES - 1)) Serial.print(',');
				}
				Serial.println(']');
			#endif

			break;
		}
	}
}

void LORA_MODULE_class::sendPayloadData(RTC_MODULE_class rtc)
{
	// Reset new Payload alert and last update to prevent forever looping messages
	_lastSystemUpdateTime = millis();
	_newpayloadAlert = false;

	// No need to send when reached send limit
	if (_sendAttempts >= SEND_ATTEMPTS)
	{
		#ifdef DEBUGGING
			Serial.println(F("Send Limit Reached!"));
		#endif

		return;
	}
	// Only sync rtc on the first attempt of sending the date & time
	else if(strncmp(_loraprevHeader, _validHeaders[DATE], sizeof(_loraprevHeader)) == 0 && _sendAttempts == 0)
	{
		#ifdef DEBUGGING
			Serial.println(F("Syncing RTC via LoRa"));
		#endif

		rtc.syncTime(_systemValues);
	}

	// Create new Payload
	char relay_message[MAX_MESSAGE_LENGTH] = {0};
	uint8_t pos = 0;
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		char numStr[MAX_NUMBER_LENGTH];
		if(_systemValues[i] == 0)
		{
			numStr[0] = BLANK_PLACEHOLDER;
    		numStr[1] = '\0';
		}
		else
		{
			uint8_t width = MAX_NUMBER_LENGTH;

			// Set width to exact amount
			if (_systemValues[i] >= 100 || _systemValues[i] <= -10)
			{
				width -= 1;
			}
			else if ((_systemValues[i] >= 10 && _systemValues[i] < 100) || (_systemValues[i] < 0 && _systemValues[i] > -10))
			{
				width -= 2;
			}
			else
			{
				width -= 3;
			}
			
			dtostrf(_systemValues[i], width, DECIMAL_VALUES, numStr);			
		}

		strcpy(&relay_message[pos], numStr);
		pos += strlen(numStr);

		if (i < MAX_DEVICES - 1) relay_message[pos++] = ',';
	}

	char sendPayload[MAX_MESSAGE_LENGTH];
	snprintf(sendPayload, sizeof(sendPayload), "%s[%s]", _loraprevHeader, relay_message);
	uint8_t payloadLen = strlen(sendPayload) + 1;

	#ifdef DEBUGGING
		Serial.print(F("Payload to Send: "));
		Serial.println(sendPayload);
	#endif

	// Encryption
	#ifdef ENCRYPTING
		char encryptedPayload[payloadLen] = {0};
		strncpy(encryptedPayload, sendPayload, payloadLen);

		// Encrypt Data
		rc4EncryptDecrypt(encryptedPayload, payloadLen);

		#ifdef DEBUGGING
			Serial.print(F("Encrypted Message: "));
			for (uint8_t i = 0; i < payloadLen; i++)
			{
				Serial.print(encryptedPayload[i]);
			}
			Serial.println();
		#endif
	#endif
	
	// CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance)
	while (_sendAttempts < SEND_ATTEMPTS)
	{
		unsigned long startTime = millis();
		unsigned long lastCheckTime = millis();

		while (millis() - startTime < _csmaTimeout)
		{
			// Check for incoming messages
			if (LoRa.parsePacket() && getLoRaPayload())
			{ 
				_newpayloadAlert = true;
				return;
			}
			else if (millis() - lastCheckTime >= _backoffTime)
			{
				lastCheckTime = millis();

				#ifdef DEBUGGING
					Serial.print('.');
				#endif
				
				// Perform backoff without blocking execution
				if (LoRa.packetRssi() < CSMA_NOISE_LIM) break;
			}

			wdt_reset();
		}

		// Send the payload over LoRa
		LoRa.beginPacket();
		#ifdef ENCRYPTING
			LoRa.write((const uint8_t*)encryptedPayload, payloadLen - 1);	// -1 Don't send null terminator
		#else
			LoRa.write((const uint8_t*)sendPayload, payloadLen - 1);		// -1 Don't send null terminator
		#endif
		LoRa.endPacket(true);												// Set true for non blocking sending

		// Debugging output for the sent message
		#ifdef DEBUGGING
			Serial.print(F("\nPayload Sent Sucessfully! ("));
			Serial.print(_sendAttempts + 1);
			Serial.print('/');
			Serial.print(SEND_ATTEMPTS);
			Serial.println(')');
		#endif

		// Increment attempts
		_sendAttempts++;
		_lastSystemUpdateTime = millis();
	}
}

#ifdef ENCRYPTING
	void LORA_MODULE_class::rc4EncryptDecrypt(char *data, uint8_t len)
	{
		// Using RC4 because originally AES was the plan but the RAM/CPU on this thing cant take it.
		// RC4 uses 256 but this code is limited to 255. (uint8_t[255] = 256 bytes | uint16_t[256] = 512 bytes)
		// Save the RAM!

		uint8_t S[RC4_BYTES];
		for (uint8_t i = 0; i < RC4_BYTES; i++)
		{
			S[i] = i;
		}

		uint8_t j = 0, temp;
		uint8_t enc_len = strlen(ENCRYPTION_KEY);
		for (uint8_t i = 0; i < RC4_BYTES; i++)
		{
			j = (j + S[i] + ENCRYPTION_KEY[i % enc_len]) % RC4_BYTES;
			temp = S[i];
			S[i] = S[j];
			S[j] = temp;
		}

		uint8_t rnd = 0, i = 0; j = 0;
		for (uint8_t n = 0; n < len - 1; n++)	// -1 dont include null terminator
		{
			i = (i + 1) % RC4_BYTES;
			j = (j + S[i]) % RC4_BYTES;
			temp = S[i];
			S[i] = S[j];
			S[j] = temp;
			rnd = S[(S[i] + S[j]) % RC4_BYTES];
			data[n] ^= rnd;
		}
	}
#endif