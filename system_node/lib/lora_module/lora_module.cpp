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
			Serial.println("LoRa Initialization Failed!");
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

void LORA_MODULE_class::loadSensorData(IDATA IData)
{
	_sensorData[TEMPERATURE] = IData.SYSTEM_TEMPERATURE;
	_sensorData[HUMIDITY] = IData.SYSTEM_TEMPERATURE;
	_sensorData[MOISTURE] = IData.SYSTEM_TEMPERATURE;
	_sensorData[BATT_VOLTAGE] = IData.SYSTEM_TEMPERATURE;

	////////////////////////////////// DELETE ME SOON
	if (_hwid == 7)
	{
		_sensorData[TEMPERATURE] = 69.420;
	}
	else if (_hwid == 1)
	{
		_sensorData[TEMPERATURE] = 11.111;
	}
	else if (_hwid == 2)
	{
		_sensorData[TEMPERATURE] = 22.222;
	}
	else if (_hwid == 3)
	{
		_sensorData[TEMPERATURE] = 33.333;
	}
	////////////////////////////////// DELETE ME SOON
}

void LORA_MODULE_class::startLoRaMesh()
{
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

	#ifdef DEBUGGING
		//	Print LoRa Payload
		Serial.println("Payload: " + _loraPayload);
	#endif

	return checkMessageValidity();
}

bool LORA_MODULE_class::checkMessageValidity()
{
	int startIdx = _loraPayload.indexOf('[');
	int endIdx = _loraPayload.indexOf(']');
	int payloadLen = _loraPayload.length();
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
	int startIdx = _loraPayload.indexOf('[');
	int endIdx = _loraPayload.indexOf(']');
	int payloadLen = _loraPayload.length();

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
	int startIdx = _loraPayload.indexOf('[');
	int endIdx = _loraPayload.indexOf(']');
	String arrayContent = _loraPayload.substring(startIdx + 1, endIdx);
	char buffer[arrayContent.length() + 1];
	arrayContent.toCharArray(buffer, arrayContent.length() + 1);
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
		if (tempValues[i] != _systemValues[i])
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
	// Reset new Payload alert
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
	relay_message.reserve(MAX_DEVICES * (DECIMAL_VALUES + 2));
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		relay_message += (_systemValues[i] == 0 ? "*" : String(_systemValues[i], DECIMAL_VALUES)) + (i < MAX_DEVICES - 1 ? "," : "");
	}
	
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

		// Send Payload
		LoRa.beginPacket();
		LoRa.print(_loraprevHeader + "[" + relay_message + "]");
		LoRa.endPacket();

		#ifdef DEBUGGING
			Serial.println("\nSends: " + String(_sendAttempts + 1) + "/" + String(SEND_ATTEMPTS));
			Serial.println("Message sent successfully: " + _loraprevHeader + "[" + relay_message + "]");
		#endif

		// Increment attempts
		_sendAttempts++;
		_lastSystemUpdateTime = millis();
	}
}
