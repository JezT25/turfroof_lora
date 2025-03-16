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

        bool regex = true;
        for (size_t i = 0; i < lora_payload.length(); i++) {
            char c = lora_payload[i];
            if (!((c >= 'A' && c <= 'Z') ||    // A-Z
                  (c >= '0' && c <= '9') ||    // 0-9
                  c == '[' || c == ']' ||      // Brackets
                  c == ',' || c == '.' || c == ':'))
            {     // Comma and period
                regex = false;
                break;  // Invalid character found
            }
        }// change this soon to more serious like first 4 must be ____ then : then [ then 0.00 comma 0.00 up to 8 then ]

        if(regex == false)
        {
            return;
        }

		if(lora_payload.startsWith("TEMP:["))
		{
			new_Payload = "TEMP:[";
			// newValues[ISystem.HW_ID] = IData.SYSTEM_TEMPERATURE;
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
            // todo hoy
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
        if (relay_data)
        {
            sends = 0;
        }
		
		// Data has updated and needs to be resent
        // if (relay_data)
        if (sends < 1)
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
			// uint8_t backoffTime = random(BACKOFF_MIN, BACKOFF_MAX) * (1 + ISystem.HW_ID/10);
            // uint8_t backoffTime = random(BACKOFF_MIN, BACKOFF_MAX) + (ISystem.HW_ID * 95);
            int backoffTime = ISystem.HW_ID * 75;
			

            Serial.print("Backoff Time: ");
            Serial.println(backoffTime);

            while(sends < 1)
            {
                unsigned long startTime = millis();
			    unsigned long lastCheckTime = millis();

                // while (millis() - startTime < random(CSMA_TOUT_MIN, CSMA_TOUT_MAX))
                while (millis() - startTime < (3000 + (500 * ISystem.HW_ID)))
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
                            Serial.println("New Data: " + lora_payload);
                            Serial.println("New Data Available. Resetting");
                            new_incomingPayload = true;
                            break;
                        }
                    }
                    else
                    {
                        // Perform backoff without blocking execution
                        if (millis() - lastCheckTime >= backoffTime)
                        {
                            // Serial.print("Noise: ");
                            // Serial.print(LoRa.packetRssi());
                            // Serial.print(" dBm | ");
                            Serial.print(".");
                            // if(LoRa.packetRssi() < CSMA_NOISE_LIM) break;
                            if(LoRa.packetRssi() < -90) break;

                            lastCheckTime = millis();
                            // backoffTime = random(BACKOFF_MIN, BACKOFF_MAX);
                        }
                    }
                }
                Serial.println();

                if(!new_incomingPayload)
                {
                    Serial.println("Sends: " + String(sends + 1) + "/" + String(1));
                    LoRa.beginPacket();
                    LoRa.print(new_Payload + relay_message + "]");
                    LoRa.endPacket();

                    Serial.println("Message sent successfully: TEMP:[" + relay_message + "]");
                    sends++;
                }
                else
                {
                    break;
                }
            }
        }
		else
		{
			Serial.println("No need to resend data");
		}
    }
}
