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
    if (!LoRa.begin(FREQUENCY)) {
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
		if (!LoRa.parsePacket() && !new_incomingPayload) continue;
		getPayloadData();
		if(!checkMessageValidity()) return;
		if(!preloadMessageData(IData, ISystem)) return;
		processPayloadData(ISystem);
		sendPayloadData(ISystem);
	}
}

void LORA_MODULE_class::resetValues()
{
	num_attempts = 0;
	lora_payload = "";
	lora_previousRequest = "";
	new_incomingPayload = false;
	memset(systemValues, 0, sizeof(systemValues));
	memset(newValues, 0, sizeof(newValues));

	lastSystemUpdateTime = millis();
}

void LORA_MODULE_class::getPayloadData()
{
	#ifdef DEBUG_ON
		//	Display Signal Strength (RSSI) of the Received Packet
		Serial.println("Packet received!");
		Serial.print("Signal strength [RSSI] (dBm): ");
		Serial.println(LoRa.packetRssi());
	#endif

	//	Get Payload Content 
	if(!new_incomingPayload)
	{
		lora_payload = "";
		while (LoRa.available())
		{
			lora_payload += (char)LoRa.read();
		}
	}
	new_incomingPayload = false;

	#ifdef DEBUG_ON
		//	Print LoRa Payload
		Serial.println("Payload: " + lora_payload);

		//	Print Current Database
		Serial.print("Current Data: [");
		for (uint8_t i = 0; i < MAX_DEVICES; i++)
		{
			Serial.print(systemValues[i], DECIMAL_VALUES);
			if(i < (MAX_DEVICES - 1)) Serial.print(",");
		}
		Serial.println("]");
	#endif
}

bool LORA_MODULE_class::checkMessageValidity()
{
    // Check message for invalid chars
    for(size_t i = 0; i < lora_payload.length(); i++)
    {
        char c = lora_payload[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '[' || c == ']' || c == ',' || c == '.' || c == ':' || c == '-'))
        {
            return false;
        }
    }
    return true;
}

bool LORA_MODULE_class::preloadMessageData(IDATA IData, ISYSTEM ISystem)
{
	// Clear newValues to accomodate Payload data
	memset(newValues, 0, sizeof(newValues));

	// Load Value
	if(lora_payload.startsWith("TEMP:["))
	{
		newValues[ISystem.HW_ID] = IData.SYSTEM_TEMPERATURE;
	}
	else if(lora_payload.startsWith("HUMI:["))
	{
		newValues[ISystem.HW_ID] = IData.SYSTEM_HUMIDITY;
	}
	else if(lora_payload.startsWith("SOIL:["))
	{
		newValues[ISystem.HW_ID] = IData.SOIL_MOISTURE;
	}
	else
	{
		return false;
	}

	////////////////////////////////// DELETE ME SOON
	if(ISystem.HW_ID == 7)
	{
		newValues[ISystem.HW_ID] = 69.420;
	}
	else if (ISystem.HW_ID == 1)
	{
		newValues[ISystem.HW_ID] = 11.111;
	}
	else if(ISystem.HW_ID == 2)
	{
		newValues[ISystem.HW_ID] = 22.222;
	}
	else if(ISystem.HW_ID == 3)
	{
		newValues[ISystem.HW_ID] = 33.333;
	}
	////////////////////////////////// DELETE ME SOON

	return true;
}

void LORA_MODULE_class::processPayloadData(ISYSTEM ISystem)
{
	float tempValues[MAX_DEVICES] = {};
	uint8_t startIdx = lora_payload.indexOf('[');
	uint8_t endIdx = lora_payload.indexOf(']');

	if (lora_payload.substring(0, 6) != lora_previousRequest || startIdx == endIdx - 1)	// Wipe systemValues if fresh request (e.g. [])
	{
		memset(systemValues, 0, sizeof(systemValues));
	}
	else	// Check if Payload has existing data
	{
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
	}

	// Get new request
	lora_previousRequest = lora_payload.substring(0, 6);

	//	Merge current values with data from Payload
	for(uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		if(i == ISystem.HW_ID) continue;
		newValues[i] = systemValues[i] == 0 ? (tempValues[i] == 0 ? 0 : tempValues[i]) : systemValues[i];
	}

	#ifdef DEBUG_ON
		//	Print Merged Values
		Serial.print("Merged Data: [");
		for (uint8_t i = 0; i < MAX_DEVICES; i++)
		{
			Serial.print(newValues[i], DECIMAL_VALUES);
			if(i < (MAX_DEVICES - 1)) Serial.print(",");
		}
		Serial.println("]");
	#endif
}

void LORA_MODULE_class::sendPayloadData(ISYSTEM ISystem)
{
	// Check if data needs to be relayed
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		if(newValues[i] != systemValues[i])
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

	// Copy newValues into systemValues
	memcpy(systemValues, newValues, sizeof(systemValues));

	// Preallocate buffer for efficient concatenation
	String relay_message;
	relay_message.reserve(MAX_DEVICES * (DECIMAL_VALUES + 2)); // Approximate size
	for (uint8_t i = 0; i < MAX_DEVICES; i++)
	{
		relay_message += String(systemValues[i], DECIMAL_VALUES);
		if (i < MAX_DEVICES - 1)
		{
			relay_message += ",";
		}
	}
	
	// CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance)
	while(num_attempts < SEND_ATTEMPTS)
	{
		unsigned long startTime = millis();
		unsigned long lastCheckTime = millis();

		while (millis() - startTime < csmaTimeout)
		{
			// Check for incoming messages
			if (LoRa.parsePacket())
			{ 
				lora_payload = "";
				while (LoRa.available())
				{
					lora_payload += (char)LoRa.read();
				}

				if(lora_payload.startsWith("TEMP:[") || lora_payload.startsWith("HUMI:[") || lora_payload.startsWith("SOIL:["))
				{
					#ifdef DEBUG_ON
						Serial.println("\nNew Data: " + lora_payload);
						Serial.println("New Data Available. Resetting");
					#endif

					new_incomingPayload = true;
					break;
				}
			}
			else
			{
				// Perform backoff without blocking execution
				if (millis() - lastCheckTime >= backoffTime)
				{
					#ifdef DEBUG_ON
						Serial.print(".");
					#endif

					if(LoRa.packetRssi() < CSMA_NOISE_LIM) break;
					lastCheckTime = millis();
				}
			}
		}
		
		if(new_incomingPayload) break;

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
