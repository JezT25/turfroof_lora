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

#ifndef lora_module_h
#define lora_module_h

#include "system_node.hpp"

// LoRa Pins
#define LORA_RST			D1
#define LORA_NSS			D8

// LoRa Settings
#define FREQUENCY			433E6  // 433 MHz
#define TX_POWER			20     // dBm
#define BANDWIDTH			125E3  // 125 kHz
#define SYNC_WORD			0x12
#define SPREAD_FACTOR   	10
#define CODING_RATE     	5
#define PREAMBLE        	8
#define LORA_REQ_TIMEOUT	40000
#define LORA_REQ_RESEND_DIV	3

// CSMA/CA Settings
#define SEND_ATTEMPTS		2
#define BACKOFF_MUL			37
#define CSMA_NOISE_LIM		-90
#define CSMA_TOUT_MUL		250
#define CSMA_TOUT_MIN		1500

// Algorithm Settings
#define CHECKSUM			8
#define START_OF_BRACKET	5
#define DECIMAL_VALUES  	5
#define REQUEST_LENGTH		8
#define MAX_HEADER_LENGTH	6		// "TEMP:" is 5 actually, but +1 for null terminator
#define MAX_NUMBER_LENGTH	10		// "000.00000" is 9, but +1 for null terminator
#define MAX_MESSAGE_LENGTH  97		// "TEMP:[000.00000,000.00000,000.00000,000.00000,000.00000,000.00000,000.00000,000.00000,000.00000]" is 96, but + 1 for null terminator
#define EPSILON         	0.0001
#define BLANK_PLACEHOLDER	'*'
#define DELAY_SMALL 		50
#define DELAY_DIVIDER		2

class LORA_MODULE_class
{
	private:
		enum _LoRaCommands : uint8_t
		{
			TEMPERATURE,
			HUMIDITY,
			SOIL_TEMPERATURE,
			SOIL_MOISTURE,
			BATT_VOLTAGE,
			DATE,
			VALID_HEADERS
		};
		const char* _validHeaders[VALID_HEADERS] =
		{
			"TEMP:",
			"HUMI:",
			"STMP:",
			"SMOI:",
			"BATT:",
			"DATE:"
		};

		bool _newpayloadAlert;
		bool _hasNodeReplied;
		uint8_t _hwid;
		uint8_t _sendAttempts;
		uint16_t _backoffTime;
		uint16_t _csmaTimeout;
		unsigned long _lastSystemUpdateTime;
		float _systemValues[MAX_DEVICES];
		char _loraPayload[MAX_MESSAGE_LENGTH];
		char _loraprevHeader[MAX_HEADER_LENGTH];

		int8_t getcharIndex(char c);
		float *getpayloadValues();
		void resetValues();
		bool getLoRaPayload(uint8_t current_header_index);
		bool checkMessageValidity(uint8_t current_header_index);
		void processPayloadData();
		void sendPayloadData(uint8_t current_header_index);
		void sendRequest(uint8_t index, IOT_class *iot);
		bool checkComplete();
		void logData(IDATA *IData, uint8_t index);

		#ifdef ENCRYPTING
			void rc4EncryptDecrypt(char *data, uint8_t len);
		#endif

	public:
		void Initialize();
		void startLoRaMesh(IDATA *IData, IOT_class *iot);
};

#endif
