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

bool LORA_MODULE_class::Initialize(IDATA IData)
{	
	// Configure Pins
    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DI0);

	// Start LoRa
    if (!LoRa.begin(FREQUENCY))
	{
		#ifdef DEBUGGING
			Serial.println("ERROR: LoRa Initialization Failed!");
		#endif

		return false;
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
		Serial.print("Backoff Time (ms): ");
		Serial.println(_backoffTime);
		Serial.print("CSMA Timeout (ms): ");
		Serial.println(_csmaTimeout);
    	Serial.println("LoRa Setup Complete!");
	#endif

    return true;
}

void LORA_MODULE_class::loadsensorData(IDATA IData)
{
	_sensorData[TEMPERATURE] = IData.SYSTEM_TEMPERATURE;
	_sensorData[HUMIDITY] = IData.SYSTEM_HUMIDITY;
	_sensorData[SOIL_TEMPERATURE] = IData.SOIL_TEMPERATURE;
	_sensorData[SOIL_MOISTURE] = IData.SOIL_MOISTURE;
	_sensorData[BATT_VOLTAGE] = IData.BATTERY_VOLTAGE;
}

void LORA_MODULE_class::startLoRaMesh(IDATA IData)
{
	loadsensorData(IData);
	resetValues();

	while (millis() - _lastSystemUpdateTime <= LORA_WAKE_TIMEOUT)
	{
		if (LoRa.parsePacket() || _newpayloadAlert)
		{
			if (!_newpayloadAlert && !getLoRaPayload()) continue;
			preloadMessageData();
			processPayloadData();
			sendPayloadData();
		}
	}
}

void LORA_MODULE_class::resetValues()
{
	// Reset values for fresh requests
	_sendAttempts = 0;
	_loraPayload = "";
	_loraprevHeader = "";
	_newpayloadAlert = false;	
	_lastSystemUpdateTime = millis();
	memset(_systemValues, 0, sizeof(_systemValues));
}

bool LORA_MODULE_class::getLoRaPayload()
{
	#ifdef DEBUGGING
		//	Display Signal Strength (RSSI) of the Received Packet
		Serial.println("Packet received!");
		Serial.print("Signal strength [RSSI] (dBm): ");
		Serial.println(LoRa.packetRssi());
	#endif

	//	Get Payload content 
	_loraPayload = "";
	while (LoRa.available())
	{
		_loraPayload += (char)LoRa.read();
	}

	#ifdef ENCRYPTING
		#ifdef DEBUGGING
			Serial.println("Encrypted Payload: " + _loraPayload);
		#endif

		uint8_t lora_len = _loraPayload.length() + 1; // +1 space for null terminator
		char decryptedPayload[lora_len];
		_loraPayload.toCharArray(decryptedPayload, lora_len);

		rc4EncryptDecrypt(decryptedPayload, lora_len);

		_loraPayload = String(decryptedPayload);	
	#endif

	#ifdef DEBUGGING
		//	Print LoRa Payload
		Serial.println("Payload: " + _loraPayload);
	#endif

	return checkMessageValidity();
}

bool LORA_MODULE_class::checkMessageValidity()
{
	uint8_t startIdx = _loraPayload.indexOf('[');
	uint8_t endIdx = _loraPayload.indexOf(']');
	uint8_t payloadLen = _loraPayload.length();
	bool checkforCharacters = true;

	// Check header validity
	for (uint8_t i = 0; i < VALID_HEADERS; i++)
	{
        if (_loraPayload.startsWith(_validHeaders[i])) break;

		if(i == VALID_HEADERS - 1)
		{
			#ifdef DEBUGGING
				Serial.println("Invalid Packet Header!");
			#endif

			return false;
		}
    }

	// Check Payload data brackets
    if (startIdx == -1 && endIdx == -1)
	{
		#ifdef DEBUGGING
			Serial.println("Unable to Locate Data!");
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
					Serial.print("Invalid Character Found!");
				#endif

				return false;
			}

			if ((payloadChar == '[' && _loraPayload[i + 2] == ',') || (payloadChar == ',' && _loraPayload[i + 2] == ',') || (payloadChar == ',' && _loraPayload[i + 2] == ']'))
			{
				if (_loraPayload[i + 1] != BLANK_PLACEHOLDER)
				{
					#ifdef DEBUGGING
						Serial.println("Invalid Data Found!");
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
					Serial.print("Invalid Number Found!");
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
	uint8_t startIdx = _loraPayload.indexOf('[');
	uint8_t endIdx = _loraPayload.indexOf(']');
	uint8_t payloadLen = _loraPayload.length();

	if ((startIdx == START_OF_BRACKET && endIdx == START_OF_BRACKET + 1 && payloadLen == START_OF_BRACKET + 2) || _loraPayload.substring(0, START_OF_BRACKET) != _loraprevHeader)
	{
		// Reset database
		memset(_systemValues, 0, sizeof(_systemValues));

		// Load values
		for (uint8_t i = 0; i < VALID_HEADERS; i++)
		{
			if (_loraPayload.startsWith(_validHeaders[i]))
			{
				_systemValues[_hwid] = _sensorData[i];
				break;
			}
		}

		// Get new header
		_loraprevHeader = _loraPayload.substring(0, START_OF_BRACKET);
	}

	#ifdef DEBUGGING
		//	Print Current Database
		Serial.print("Current Data: [");
		for (uint8_t i = 0; i < MAX_DEVICES; i++)
		{
			Serial.print(_systemValues[i], DECIMAL_VALUES);
			if (i < (MAX_DEVICES - 1)) Serial.print(",");
		}
		Serial.println("]");
	#endif
}

void LORA_MODULE_class::processPayloadData()
{
	// Extract data from Payload
	uint8_t startIdx = _loraPayload.indexOf('[');
	uint8_t endIdx = _loraPayload.indexOf(']');
	String arrayContent = _loraPayload.substring(startIdx + 1, endIdx);
	uint8_t array_len = arrayContent.length() + 1;
	char buffer[array_len];
	arrayContent.toCharArray(buffer, array_len);
	char *token = strtok(buffer, ",");

	// Store data to tempValues
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
				//	Print Merged Values
				Serial.print("Merged Data: [");
				for (uint8_t i = 0; i < MAX_DEVICES; i++)
				{
					Serial.print(_systemValues[i], DECIMAL_VALUES);
					if (i < (MAX_DEVICES - 1)) Serial.print(",");
				}
				Serial.println("]");
			#endif

			break;
		}
	}
}

void LORA_MODULE_class::sendPayloadData()
{
	// Reset new Payload alert and last update to prevent forever looping messages
	_lastSystemUpdateTime = millis();
	_newpayloadAlert = false;

	// No need to send when reached send limit
	if (_sendAttempts >= SEND_ATTEMPTS)
	{
		#ifdef DEBUGGING
			Serial.println("Send Limit Reached!");
		#endif

		return;
	}

	// Create new Payload
	String relay_message;
	relay_message.reserve(MAX_MESSAGE_LENGTH);
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		relay_message += (_systemValues[i] == 0 ? String(BLANK_PLACEHOLDER) : String(_systemValues[i], DECIMAL_VALUES)) + (i < MAX_DEVICES - 1 ? "," : "");
	}
	String sendPayload = _loraprevHeader + "[" + relay_message + "]";
	uint8_t payloadLen = sendPayload.length() + 1;	// +1 Space for null terminator

	#ifdef DEBUGGING
		Serial.println("Payload to Send: " + sendPayload);
	#endif

	// Encryption
	#ifdef ENCRYPTING
		char encryptedPayload[payloadLen] = {0};
		sendPayload.toCharArray(encryptedPayload, payloadLen);

		// Encrypt Data
		rc4EncryptDecrypt(encryptedPayload, payloadLen);

		#ifdef DEBUGGING
			Serial.print("Encrypted Message: ");
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
					Serial.print(".");
				#endif
				
				// Perform backoff without blocking execution
				if (LoRa.packetRssi() < CSMA_NOISE_LIM) break;
			}
		}

		// Send the payload over LoRa
		LoRa.beginPacket();
		#ifdef ENCRYPTING
			LoRa.write((uint8_t*)encryptedPayload, payloadLen - 1);	 			// -1 Don't send null terminator
		#else
			LoRa.write((const uint8_t*)sendPayload.c_str(), payloadLen - 1);	// -1 Don't send null terminator
		#endif
		LoRa.endPacket();

		// Debugging output for the sent message
		#ifdef DEBUGGING
			Serial.println("\nPayload Sent Sucessfully! (" + String(_sendAttempts + 1) + "/" + String(SEND_ATTEMPTS) + ")");
		#endif

		// Increment attempts
		_sendAttempts++;
		_lastSystemUpdateTime = millis();
	}
}

#ifdef ENCRYPTING
	void LORA_MODULE_class::rc4EncryptDecrypt(char *data, uint8_t len)
	{
		// Using RC4 Modified because originally AES was the plan but the RAM/CPU on this thing cant take it.
		// RC4 uses 256 bytes to randomize but that needs int. RAM/CPU is dying so switch to uint_8 which is up to 255 🤯
		// So ok we use 255, RAM/CPU still dies! Kinda works with Serial Off, but for the sake of this I will lower it to 128 or even 64.
		// Might use 255 for final since serial will be off, depends on reliability

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
		for (uint8_t n = 0; n < len - 1; n++)	// -1 dont encrypt null terminator
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