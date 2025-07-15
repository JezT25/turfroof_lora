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

#include "system_node.hpp"

void LORA_MODULE_class::Initialize()
{
	// Configure Pins
	LoRa.setPins(LORA_NSS, LORA_RST);

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
	_hwid = config::HW_ID;

	// CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance) Parameters
	_backoffTime = BACKOFF_MUL + _hwid * BACKOFF_MUL;
	_csmaTimeout = CSMA_TOUT_MIN + (CSMA_TOUT_MUL * _hwid);

	#ifdef DEBUGGING
		Serial.print(F("Backoff Time (ms): "));
		Serial.println(_backoffTime);
		Serial.print(F("CSMA Timeout (ms): "));
		Serial.println(_csmaTimeout);
		Serial.println(F("LoRa Setup Complete!"));
	#endif
}

void LORA_MODULE_class::startLoRaMesh(IDATA *IData, IOT_class *iot, int number)
{
	uint8_t startIdx = 0;
	uint8_t endIdx = VALID_HEADERS;
	if (number >= 1 && number <= 5) {
		startIdx = number - 1;
		endIdx = number - 1;
	}
	for (uint8_t i = startIdx; i < endIdx; i++)
	{
		// Reset Everything
		resetValues();

		// Send Requests
		sendRequest(i, iot);
		_hasNodeReplied = false;

		while (millis() - _lastSystemUpdateTime <= LORA_REQ_TIMEOUT)
		{
			if (LoRa.parsePacket() || _newpayloadAlert)
			{
				if (!_newpayloadAlert && !getLoRaPayload(i)) continue;
				processPayloadData();
				sendPayloadData(i);
				_hasNodeReplied = true;
			}

			if (_hasNodeReplied == false && millis() - _lastSystemUpdateTime >= LORA_REQ_TIMEOUT / LORA_REQ_RESEND_DIV)
			{
				sendRequest(i, iot);
				_hasNodeReplied = true;
			}

			// Check if all active devices have sent data
			if (checkComplete()) break;

			yield();
		}

		// Store data to IDATA
		logData(IData, i);
	}
}

void LORA_MODULE_class::logData(IDATA *IData, uint8_t index)
{
	for (uint8_t i = 0; i < MAX_DEVICES - 1; i++)
	{
		switch (index)
		{
			case TEMPERATURE:
				IData->TEMP_DATA[i] = _systemValues[i];
				break;
			case HUMIDITY:
				IData->HUMI_DATA[i] = _systemValues[i];
				break;
			case SOIL_TEMPERATURE:
				IData->STMP_DATA[i] = _systemValues[i];
				break;
			case SOIL_MOISTURE:
				IData->SMOI_DATA[i] = _systemValues[i];
				break;
			case BATT_VOLTAGE:
				IData->BATT_DATA[i] = _systemValues[i];
				break;
		}
	}
}

bool LORA_MODULE_class::checkComplete()
{
	for (uint8_t i = 0; i < MAX_DEVICES - 1; i++)
	{
		if (config::ACTIVE_DEVICES[i] && _systemValues[i] == 0)
		{
			return false;
		}
	}

	#ifdef DEBUGGING
		Serial.println(F("All active devices have reported data!"));
	#endif

	return true;
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

void LORA_MODULE_class::sendRequest(uint8_t index, IOT_class *iot)
{
	char sendPayload[index == DATE ? MAX_MESSAGE_LENGTH : REQUEST_LENGTH] = {0};
	snprintf(_loraprevHeader, sizeof(_loraprevHeader), "%s", _validHeaders[index]);

	if (index == DATE)
	{
		iot->timeClient.update();
		unsigned long epochTime = iot->timeClient.getEpochTime();
		uint8_t currentHour   = hour(epochTime);
		uint8_t currentMinute = minute(epochTime);
		uint8_t currentSecond = second(epochTime);
		uint8_t currentDay    = day(epochTime);
		uint8_t currentMonth  = month(epochTime);
		uint8_t currentYear   = year(epochTime) % 100;
		uint8_t checkSum = currentHour + currentMinute + currentSecond + currentDay + currentMonth + currentYear;

		snprintf(sendPayload, sizeof(sendPayload), "%s[%02d,%02d,%02d,%02d,%02d,%02d,*,*,%02d]",
			_validHeaders[index],
			currentHour,
			currentMinute,
			currentSecond,
			currentDay,
			currentMonth,
			currentYear,
			checkSum
		);
	}
	else
	{
		snprintf(sendPayload, sizeof(sendPayload), "%s[]", _validHeaders[index]);
	}

	#ifdef DEBUGGING
		Serial.print(F("Payload to Send: "));
		Serial.println(sendPayload);
	#endif

	uint8_t payloadSize = strlen(sendPayload) + 1;

	#ifdef ENCRYPTING
		char encryptedPayload[payloadSize] = {0};
		strncpy(encryptedPayload, sendPayload, payloadSize);
		rc4EncryptDecrypt(encryptedPayload, payloadSize);

		#ifdef DEBUGGING
			Serial.print(F("Encrypted Message: "));
			for (uint8_t i = 0; i < payloadSize - 1; i++)
			{
				Serial.print(encryptedPayload[i]);
			}
			Serial.println();
		#endif
	#endif

	for (uint8_t j = 0; j < SEND_ATTEMPTS; j++)
	{
		LoRa.beginPacket();
		#ifdef ENCRYPTING
			LoRa.write((const uint8_t*)encryptedPayload, payloadSize - 1);	// -1 Don't send null terminator
		#else
			LoRa.write((const uint8_t*)sendPayload, payloadSize - 1);		// -1 Don't send null terminator
		#endif
		LoRa.endPacket();
		#ifdef DEBUGGING
			Serial.print(F("\nPayload Sent Sucessfully! ("));
			Serial.print(j + 1);
			Serial.print('/');
			Serial.print(SEND_ATTEMPTS);
			Serial.println(')');
		#endif

		delay(_csmaTimeout / DELAY_DIVIDER);
	}
}

bool LORA_MODULE_class::getLoRaPayload(uint8_t current_header_index)
{
	#ifdef DEBUGGING
		//	Display Signal Strength (RSSI) of the Received Packet
		Serial.println(F("\nPacket received!"));
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
	payloadIndex++; // +1 as space for null terminator

	// Flush remaining bytes (if any)
	while (LoRa.available()) LoRa.read();

	#ifdef ENCRYPTING
		#ifdef DEBUGGING
			Serial.print(F("Encrypted Payload: "));
			Serial.println(_loraPayload);
		#endif

		char decryptedPayload[MAX_MESSAGE_LENGTH];
		memcpy(decryptedPayload, _loraPayload, payloadIndex);
		rc4EncryptDecrypt(decryptedPayload, payloadIndex);
		memcpy(_loraPayload, decryptedPayload, payloadIndex);
	#endif

	#ifdef DEBUGGING
		//	Print LoRa Payload
		Serial.print(F("Payload: "));
		Serial.println(_loraPayload);
	#endif

	return checkMessageValidity(current_header_index);
}

int8_t LORA_MODULE_class::getcharIndex(char c)
{
	for (uint8_t i = 0; i < sizeof(_loraPayload); i++)
	{
		if (_loraPayload[i] == c) return i;
	}
	return -1;
}

float *LORA_MODULE_class::getpayloadValues()
{
	static float tempValues[MAX_DEVICES];
	memset(tempValues, 0, sizeof(tempValues));

	uint8_t startIdx = getcharIndex('[');
	uint8_t endIdx = getcharIndex(']');
	uint8_t array_len = endIdx - (startIdx + 1);
	char buffer[array_len + 1];
	strncpy(buffer, &_loraPayload[startIdx + 1], array_len);
	buffer[array_len] = '\0';

	char* token = strtok(buffer, ",");
	uint8_t index = 0;
	while (token != nullptr && index < MAX_DEVICES)
	{
		tempValues[index++] = atof(token);
		token = strtok(nullptr, ",");
	}

	return tempValues;
}

bool LORA_MODULE_class::checkMessageValidity(uint8_t current_header_index)
{
	uint8_t startIdx = getcharIndex('[');
	uint8_t endIdx = getcharIndex(']');
	uint8_t payloadLen = strlen(_loraPayload);
	bool checkforCharacters = true;

	// Check header if it's the correct one we are looking for
	if (strncmp(_loraPayload, _validHeaders[current_header_index], strlen(_validHeaders[current_header_index])) != 0)
	{
		#ifdef DEBUGGING
			Serial.println(F("X: Invalid Packet Header!"));
		#endif

		return false;
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

	// Check for Checksum
	if(startIdx == START_OF_BRACKET && endIdx != START_OF_BRACKET + 1)
	{
		float* tempValues = getpayloadValues();

		float sum = 0;
		for (uint8_t i = 0; i < CHECKSUM; i++)
		{
			sum += tempValues[i];
		}

		#ifdef DEBUGGING
			Serial.print(F("Calculating Checksum: "));
			Serial.print(sum, DECIMAL_VALUES);
			Serial.print(F(" vs "));
			Serial.println(tempValues[CHECKSUM], DECIMAL_VALUES);
		#endif

		if (fabs(sum - tempValues[CHECKSUM]) > EPSILON)
		{
			#ifdef DEBUGGING
				Serial.println(F("Checksum is BAD!"));
			#endif

			return false;
		}
		#ifdef DEBUGGING
			else
			{
				Serial.println(F("Checksum is GOOD!"));
			}
		#endif
	}

	return true;
}

void LORA_MODULE_class::processPayloadData()
{
	// Extract data from payload
	float* tempValues = getpayloadValues();

	// Compare stored data with new data and determine if we need to resend
	for (uint8_t i = 0; i < MAX_DEVICES - 1; i++)
	{
		//	Merge current values with data from Payload only when new data is different
		if (fabs(tempValues[i] - _systemValues[i]) > EPSILON)
		{
			bool isDate = strcmp(_loraprevHeader, _validHeaders[DATE]);
			_sendAttempts = 0;
			_systemValues[CHECKSUM] = 0;

			for (uint8_t i = 0; i < MAX_DEVICES - 1; i++)
			{
				if(isDate == 0)
				{
					_systemValues[i] = tempValues[i];
				}
				else if (_systemValues[i] == 0)
				{
					_systemValues[i] = tempValues[i] == 0 ? 0 : tempValues[i];
				}

				// Update Checksum
				_systemValues[CHECKSUM] += _systemValues[i];
			}

			#ifdef DEBUGGING
				//	Print merged values
				Serial.print(F("Merged Data: ["));
				for (uint8_t i = 0; i < MAX_DEVICES; i++)
				{
					Serial.print(strcmp(_loraprevHeader, _validHeaders[SOIL_MOISTURE]) == 0 ? (uint16_t)_systemValues[i] : _systemValues[i], DECIMAL_VALUES);
					if (i < (MAX_DEVICES - 1)) Serial.print(',');
				}
				Serial.println(']');
			#endif

			break;
		}
	}
}

void LORA_MODULE_class::sendPayloadData(uint8_t current_header_index)
{
	// Reset new Payload alert and last update to prevent forever looping messages
	_newpayloadAlert = false;

	// No need to send when reached send limit
	if (_sendAttempts >= SEND_ATTEMPTS)
	{
		#ifdef DEBUGGING
			Serial.println(F("Send Limit Reached!"));
		#endif

		return;
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

			if (strcmp(_loraprevHeader, _validHeaders[SOIL_MOISTURE]) == 0)
			{
				snprintf(numStr, sizeof(numStr), "%u", (uint16_t)_systemValues[i]);
			}
			else
			{
				dtostrf(_systemValues[i], width, DECIMAL_VALUES, numStr);
			}
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
			if (LoRa.parsePacket() && getLoRaPayload(current_header_index))
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

			yield();
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