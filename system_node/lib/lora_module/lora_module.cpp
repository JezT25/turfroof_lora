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

bool LORA_MODULE_class::Initialize()
{
    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DI0);
    
    LoRa.begin(FREQUENCY);

    LoRa.setTxPower(TX_POWER);
    LoRa.setSignalBandwidth(BANDWIDTH);
    LoRa.setSyncWord(SYNC_WORD);
    LoRa.setSpreadingFactor(SPREAD_FACTOR);
    LoRa.setCodingRate4(CODING_RATE);
    LoRa.setPreambleLength(PREAMBLE);

    Serial.println("LoRa setup complete.");

    return 0;
}

void LORA_MODULE_class::getMessages(IDATA IData, ISYSTEM ISystem)
{
    if (LoRa.parsePacket() || new_incomingPayload)
    {
		/***************************************************************
            Display Signal Strength (RSSI) of the Received Packet
        ****************************************************************/
		Serial.println("Packet received!");
		Serial.print("Signal strength (RSSI): ");
		Serial.print(LoRa.packetRssi());
		Serial.println(" dBm");

		if(!new_incomingPayload)
		{
			/****************************************************************
                                Get Payload Content 
        	****************************************************************/
			lora_payload = "";
            while (LoRa.available())
            {
                lora_payload += (char)LoRa.read();
            }
		}
		new_incomingPayload = false;

		/****************************************************************
								Parse Request 
		****************************************************************/
		float newValues[MAX_DEVICES] = {};
		String new_Payload;

		if(lora_payload.startsWith("TEMP:["))
		{
			new_Payload = "TEMP:[";
			newValues[ISystem.HW_ID] = IData.SYSTEM_TEMPERATURE;
		}
		else if(lora_payload.startsWith("HUMI:["))
		{
			new_Payload = "HUMI:[";
			newValues[ISystem.HW_ID] = IData.SYSTEM_HUMIDITY;
		}
		else if(lora_payload.startsWith("SOIL:["))
		{
			new_Payload = "SOIL:[";
			newValues[ISystem.HW_ID] = IData.SOIL_MOISTURE;
		}
		else
		{
			return;
		}
        
        /* Print Current Database*/
		Serial.print("Current Data: [");
        for (int i = 0; i < MAX_DEVICES; i++)
		{
            Serial.print(myData[i], DECIMAL_VALUES);
            Serial.print(",");
        }
        Serial.println("]");

		/* Print LoRa Payload*/
        Serial.println("Payload: " + lora_payload);

        /****************************************************************
                                Get Payload Data 
        ****************************************************************/
	   	float tempValues[MAX_DEVICES] = {};
		uint8_t startIdx = lora_payload.indexOf('[');
		uint8_t endIdx = lora_payload.indexOf(']');

        if(endIdx != startIdx + 1)	// Check if Payload has existing data
        {
            String arrayContent = lora_payload.substring(startIdx + 1, endIdx);

            // Parse the numbers into the float array
            uint8_t index = 0;
            char buffer[arrayContent.length() + 1];
            arrayContent.toCharArray(buffer, arrayContent.length() + 1);

            char *token = strtok(buffer, ",");
            while (token != nullptr && index < MAX_DEVICES)
            {
                tempValues[index] = atof(token);  // Convert string to float
                index++;
                token = strtok(nullptr, ",");
            }
        }

        /****************************************************************
                        Merge Payload with Current Data
        ****************************************************************/
        for(int i = 0; i < MAX_DEVICES; i++)
        {
            if(i == ISystem.HW_ID) continue;
            newValues[i] = myData[i] == 0 ? (tempValues[i] == 0 ? 0 : tempValues[i]) : myData[i];
        }

		/* Print Merged Values*/
        Serial.print("Merged Data: [");
        for (int i = 0; i < MAX_DEVICES; i++)
		{
            Serial.print(newValues[i], DECIMAL_VALUES);
            Serial.print(",");
        }
        Serial.println("]");

        /****************************************************************
                Determine if Data is NEW and needs to be relayed
        ****************************************************************/
	   	bool relay_data = false;
	   	for (int i = 0; i < MAX_DEVICES && !relay_data; i++)
		{
			relay_data = newValues[i] != myData[i];
		}
		
		// Data has updated and needs to be resent
        if (relay_data)
        {
            // Copy newValues into myData
            memcpy(myData, newValues, sizeof(myData));

            // Preallocate buffer for efficient concatenation
            String relay_message;
            relay_message.reserve(MAX_DEVICES * (DECIMAL_VALUES + 2)); // Approximate size

            for (int i = 0; i < MAX_DEVICES; i++)
			{
                relay_message += String(myData[i], DECIMAL_VALUES);
                if (i < MAX_DEVICES - 1)
				{
                    relay_message += ",";
                }
            }
            
            // CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance)
			uint8_t backoffTime = random(BACKOFF_MIN, BACKOFF_MAX);
			uint16_t startTime = millis() & 0xFFFF;
			uint16_t lastCheckTime = 0;

			while (millis() - startTime < CSMA_TIMEOUT)
			{
                // Check for incoming messages
                if (LoRa.available())
				{ 
                    lora_payload = "";
                    while (LoRa.available())
					{
                        lora_payload += (char)LoRa.read();
                    }

                    if(lora_payload.startsWith("TEMP:[") || lora_payload.startsWith("HUMI:[") || lora_payload.startsWith("SOIL:["))
                    {
                        new_incomingPayload = true;
                        break;
                    }
                }
            
                // Perform backoff without blocking execution
                if (millis() - lastCheckTime >= backoffTime)
				{
					Serial.println(LoRa.packetRssi());
					if(LoRa.packetRssi() < CSMA_NOISE_LIM) break;

                    lastCheckTime = millis() & 0xFFFF;
                    backoffTime = random(BACKOFF_MIN, BACKOFF_MAX);
                }
            }

            if(!new_incomingPayload)
            {
                LoRa.beginPacket();
                LoRa.print(new_Payload + relay_message + "]");
                LoRa.endPacket();

                Serial.println("Message sent successfully: TEMP:[" + relay_message + "]");
            }
        }
		else
		{
			Serial.println("No need to resend data");
		}
    }
}
