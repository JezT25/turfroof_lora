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
    #ifdef LORA_SERIAL
        Serial.begin(SERIAL_BAUD);
        // Todo: while (!Serial);
    #endif

    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DI0);
    
    if (!LoRa.begin(FREQUENCY)) {
        // TODO
        Serial.println("LoRa init failed.");
        while (1);
    }

    LoRa.setTxPower(TX_POWER);
    LoRa.setSignalBandwidth(BANDWIDTH);
    LoRa.setSyncWord(SYNC_WORD);
    LoRa.setSpreadingFactor(SPREAD_FACTOR);
    LoRa.setCodingRate4(CODING_RATE);
    LoRa.setPreambleLength(PREAMBLE);

    #ifdef LORA_SERIAL
        Serial.println("LoRa setup complete.");
    #endif

    return 0;
}

unsigned long startTime = millis();
const unsigned long CSMA_TIMEOUT = 5000; // Timeout after 5 seconds
unsigned long lastCheckTime = 0; 
const unsigned long BACKOFF_MIN = 50; 
const unsigned long BACKOFF_MAX = 200; 
unsigned long backoffTime = random(BACKOFF_MIN, BACKOFF_MAX);
bool new_incomingPayload = false;
String receivedMessage = "";

void LORA_MODULE_class::getMessages(IDATA IData, ISYSTEM ISystem)
{
    if (LoRa.parsePacket() || new_message)
    {
        /***************************************************************
            Display Signal Strength (RSSI) of the Received Packet
        ****************************************************************/
        // Serial.println("Packet received!");
        // Serial.print("Signal strength (RSSI): ");
        // Serial.print(LoRa.packetRssi());
        // Serial.println(" dBm");

        /****************************************************************
                                Get Payload Content 
        ****************************************************************/
        float newValues[MAX_DEVICES] = {};
        String payload;

        if(false == new_message)
        {
            payload = "";
            while (LoRa.available())
            {
                payload += (char)LoRa.read();
            }

            /****************************************************************
                                    Parse Request 
            ****************************************************************/
            

            if(payload.startsWith("TEMP:"))
            {
                newValues[ISystem.HW_ID] = IData.SYSTEM_TEMPERATURE;
            }
            else if(payload.startsWith("HUMI:"))
            {
                newValues[ISystem.HW_ID] = IData.SYSTEM_HUMIDITY;
            }
            else if(payload.startsWith("SOIL:"))
            {
                newValues[ISystem.HW_ID] = IData.SOIL_MOISTURE;
            }
            else
            {
                return;
            }
        }
        else
        {
            payload = receivedMessage;
        }
        new_message = false;

        //TODO
        Serial.println(payload);

        /****************************************************************
                                Get Payload Data 
        ****************************************************************/
        float tempValues[MAX_DEVICES] = {};
        uint8_t startIdx = payload.indexOf('[');
        uint8_t endIdx = payload.indexOf(']');

        // If Payload has existing data
        if(endIdx != startIdx + 1)
        {
            String arrayContent = payload.substring(startIdx + 1, endIdx);

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

        //TODO
        Serial.print("MERGED values: ");
        for (int i = 0; i < MAX_DEVICES; i++) {
            Serial.print(newValues[i], DECIMAL_VALUES);
            Serial.print(" ");
        }
        Serial.println();

        /****************************************************************
                Determine if Data is NEW and needs to be relayed
        ****************************************************************/
        bool relay_data = false;

        for(int i = 0; i < MAX_DEVICES; i++)
        {
            if(newValues[i] != myData[i])
            {
                relay_data = true;
                break;
            }
        }

        if (true == relay_data)
        {
            // Copy newValues into myData
            memcpy(myData, newValues, sizeof(myData));

            // Preallocate buffer for efficient concatenation
            String relay_message;
            relay_message.reserve(MAX_DEVICES * (DECIMAL_VALUES + 2)); // Approximate size

            for (int i = 0; i < MAX_DEVICES; i++) {
                relay_message += String(myData[i], DECIMAL_VALUES);
                if (i < MAX_DEVICES - 1) {
                    relay_message += ",";
                }
            }
            
            // CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance)
            while (LoRa.packetRssi() > -90) { // While the channel is busy
                // Check for incoming messages
                if (LoRa.available()) { 
                    receivedMessage = "";
                    while (LoRa.available()) {
                        receivedMessage += (char)LoRa.read();
                    }

                    if(receivedMessage.startsWith("TEMP:") || receivedMessage.startsWith("HUMI:") || receivedMessage.startsWith("SOIL:"))
                    {
                        new_message = true;
                        break;
                    }
                }
            
                // Perform backoff without blocking execution
                if (millis() - lastCheckTime >= backoffTime) {
                    lastCheckTime = millis();  // Reset check time
                    backoffTime = random(BACKOFF_MIN, BACKOFF_MAX);  // Randomize next check interval
                }
            
                // Timeout check
                if (millis() - startTime > CSMA_TIMEOUT) {
                    Serial.println("CSMA Timeout! Sending message anyway.");
                    break; // Exit loop and send message
                }
            }

            if(false == new_message)
            {
                // Send the formatted message
                LoRa.beginPacket();
                LoRa.print("TEMP:[" + relay_message + "]");
                LoRa.endPacket();

                // Debugging output
                Serial.println("Message sent successfully: TEMP:[" + relay_message + "]");
            }
        }
    }
}
