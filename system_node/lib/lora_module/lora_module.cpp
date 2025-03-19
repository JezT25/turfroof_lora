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

bool LORA_MODULE_class::Initialize(ISYSTEM ISystem)
{	
	// Configure Pins
    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DI0);

	// Start LoRa
    if (!LoRa.begin(FREQUENCY))
	{
		#ifdef DEBUG_ON
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

	// CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance) Parameters
	backoffTime = ISystem.HW_ID * BACKOFF_MUL;
	csmaTimeout = CSMA_TOUT_MIN + (CSMA_TOUT_MUL * ISystem.HW_ID);

	#ifdef DEBUG_ON
		Serial.print("Backoff Time (ms): ");
		Serial.println(backoffTime);
		Serial.print("CSMA Timeout (ms): ");
		Serial.println(csmaTimeout);
    	Serial.println("LoRa Setup Complete!");
	#endif

    return true;
}

void LORA_MODULE_class::getMessages(IDATA IData, ISYSTEM ISystem)
{
	resetValues();

	while(millis() - lastSystemUpdateTime <= LORA_WAKE_TIMEOUT)
	{
		if(LoRa.parsePacket() || new_incomingPayload)
		{
			if(!new_incomingPayload && !getPayloadData()) continue;
			preloadMessageData(IData, ISystem);
			processPayloadData(ISystem);
			sendPayloadData(ISystem);
		}
	}

	// rework functions
}

void LORA_MODULE_class::resetValues()
{
	num_attempts = 0;
	lora_payload = "";
	lora_previousRequest = "";
	new_incomingPayload = false;
	memset(systemValues, 0, sizeof(systemValues));

	lastSystemUpdateTime = millis();
}

bool LORA_MODULE_class::getPayloadData()
{
	#ifdef DEBUG_ON
		//	Display Signal Strength (RSSI) of the Received Packet
		Serial.println("Packet received!");
		Serial.print("Signal strength [RSSI] (dBm): ");
		Serial.println(LoRa.packetRssi());
	#endif

	//	Get Payload Content 
	lora_payload = "";
	while (LoRa.available())
	{
		lora_payload += (char)LoRa.read();
	}

	#ifdef DEBUG_ON
		//	Print LoRa Payload
		Serial.println("Payload: " + lora_payload);
	#endif

	return checkMessageValidity();
}

bool LORA_MODULE_class::checkMessageValidity()
{
	if(!lora_payload.startsWith("TEMP:[") && !lora_payload.startsWith("HUMI:[") && !lora_payload.startsWith("SOIL:["))
	{
		#ifdef DEBUG_ON
			Serial.println("Invalid Packet Header!");
		#endif

		return false;
	}
    if(lora_payload.indexOf('[') == -1 && lora_payload.indexOf(']') == -1)
	{
		#ifdef DEBUG_ON
			Serial.println("Unable to Locate Data!");
		#endif

		return false;
	}

	if (lora_payload[5] == '[' && lora_payload[6] == ']' && lora_payload.length() == 7) return true;

	bool numbermode = false;

    for(size_t i = 5; i < lora_payload.length(); i++)
    {
        char c = lora_payload[i];

		if(!numbermode)
		{
			if(c == '*')
			{
				continue;
			}
			else if (!(c == '[' || c == ']' || c == ','))
			{
				#ifdef DEBUG_ON
					Serial.print("Invalid Character Found at ");
					Serial.println(i);
				#endif

				return false;
			}

			if ((c == '[' && (lora_payload[i + 2] == ',') || (c == ',' && lora_payload[i + 2] == ',') || (c == ',' && lora_payload[i + 2] == ']')))
			{
				if(lora_payload[i + 1] != '*')
				{
					#ifdef DEBUG_ON
						Serial.println("Invalid Data Found!");
					#endif

					return false;
				}
			}
			else
			{
				numbermode = true;
			}
		}
		else
		{
			if (!((c >= '0' && c <= '9') || c == '.' || c == '-'))
			{
				#ifdef DEBUG_ON
					Serial.print("Invalid Number Found at ");
					Serial.println(i);
				#endif

				return false;
			}

			if(lora_payload[i + 1] == ',' || lora_payload[i + 1] == ']')
			{
				numbermode = false;
			}
		}
    }
    return true;
}

bool LORA_MODULE_class::preloadMessageData(IDATA IData, ISYSTEM ISystem)
{
	uint8_t startIdx = lora_payload.indexOf('[');
	uint8_t endIdx = lora_payload.indexOf(']');

	new_incomingPayload = false;

	if (lora_payload.substring(0, 6) != lora_previousRequest || startIdx == endIdx - 1)	// Wipe systemValues if fresh request (e.g. [])
	{
		memset(systemValues, 0, sizeof(systemValues));

		// Load Value
		if(lora_payload.startsWith("TEMP:["))
		{
			systemValues[ISystem.HW_ID] = IData.SYSTEM_TEMPERATURE;
		}
		else if(lora_payload.startsWith("HUMI:["))
		{
			systemValues[ISystem.HW_ID] = IData.SYSTEM_HUMIDITY;
		}
		else if(lora_payload.startsWith("SOIL:["))
		{
			systemValues[ISystem.HW_ID] = IData.SOIL_MOISTURE;
		}

		////////////////////////////////// DELETE ME SOON
		{
			if(ISystem.HW_ID == 7)
			{
				systemValues[ISystem.HW_ID] = 69.420;
			}
			else if (ISystem.HW_ID == 1)
			{
				systemValues[ISystem.HW_ID] = 11.111;
			}
			else if(ISystem.HW_ID == 2)
			{
				systemValues[ISystem.HW_ID] = 22.222;
			}
			else if(ISystem.HW_ID == 3)
			{
				systemValues[ISystem.HW_ID] = 33.333;
			}
		}
		////////////////////////////////// DELETE ME SOON

		// Get new request
		lora_previousRequest = lora_payload.substring(0, 6);
	}

	#ifdef DEBUG_ON
		//	Print Current Database
		Serial.print("Current Data: [");
		for (uint8_t i = 0; i < MAX_DEVICES; i++)
		{
			Serial.print(systemValues[i], DECIMAL_VALUES);
			if(i < (MAX_DEVICES - 1)) Serial.print(",");
		}
		Serial.println("]");
	#endif

	return true;
}

void LORA_MODULE_class::processPayloadData(ISYSTEM ISystem)
{
	float tempValues[MAX_DEVICES] = {};
	uint8_t startIdx = lora_payload.indexOf('[');
	uint8_t endIdx = lora_payload.indexOf(']');

	String arrayContent = lora_payload.substring(startIdx + 1, endIdx);

	// Parse the numbers into the float array
	uint8_t index = 0;
	char buffer[arrayContent.length() + 1];
	arrayContent.toCharArray(buffer, arrayContent.length() + 1);
	char *token = strtok(buffer, ",");
	while (token != nullptr && index < MAX_DEVICES)
	{
		tempValues[index] = atof(token);
		index++;
		token = strtok(nullptr, ",");
	}

	// Check if data needs to be relayed
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		if(tempValues[i] != systemValues[i])
		{
			num_attempts = 0;
			break;
		}
	}

	if (num_attempts >= SEND_ATTEMPTS)
	{
		#ifdef DEBUG_ON
			Serial.println("No need to resend data");
		#endif

		return;
	}

	//	Merge current values with data from Payload
	for(uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		if(i == ISystem.HW_ID) continue;
		systemValues[i] = systemValues[i] == 0 ? (tempValues[i] == 0 ? 0 : tempValues[i]) : systemValues[i];
	}

	#ifdef DEBUG_ON
		//	Print Merged Values
		Serial.print("Merged Data: [");
		for (uint8_t i = 0; i < MAX_DEVICES; i++)
		{
			Serial.print(systemValues[i], DECIMAL_VALUES);
			if(i < (MAX_DEVICES - 1)) Serial.print(",");
		}
		Serial.println("]");
	#endif
}

void LORA_MODULE_class::sendPayloadData(ISYSTEM ISystem)
{
	if (num_attempts >= SEND_ATTEMPTS)	return;

	// Preallocate buffer for efficient concatenation
	String relay_message;
	relay_message.reserve(MAX_DEVICES * (DECIMAL_VALUES + 2)); // Approximate size // todo figure this out
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		if (systemValues[i] == 0)
		{
			relay_message += "*";
		}
		else
		{
			relay_message += String(systemValues[i], DECIMAL_VALUES);
		}
		if (i < MAX_DEVICES - 1) relay_message += ",";
	}
	
	// CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance)
	while(num_attempts < SEND_ATTEMPTS)
	{
		unsigned long startTime = millis();
		unsigned long lastCheckTime = millis();

		while (millis() - startTime < csmaTimeout)
		{
			// Check for incoming messages
			if (LoRa.parsePacket() && getPayloadData())
			{ 
				new_incomingPayload = true;
				return;
			}
			else if(millis() - lastCheckTime >= backoffTime)
			{
				// Perform backoff without blocking execution
				#ifdef DEBUG_ON
					Serial.print(".");
				#endif

				if(LoRa.packetRssi() < CSMA_NOISE_LIM) break;
				lastCheckTime = millis();
			}
		}

		LoRa.beginPacket();
		LoRa.print(lora_previousRequest + relay_message + "]");
		LoRa.endPacket();

		#ifdef DEBUG_ON
			Serial.println("\nSends: " + String(num_attempts + 1) + "/" + String(SEND_ATTEMPTS));
			Serial.println("Message sent successfully: " + lora_previousRequest + relay_message + "]");
		#endif

		// Increment attempts
		num_attempts++;
		lastSystemUpdateTime = millis();
	}
}
