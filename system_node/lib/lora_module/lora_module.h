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

#include "../system_node.hpp"

// LoRa Pins
#define LORA_DI0			2
#define LORA_RST			9
#define LORA_NSS			4

// LoRa Settings
#define FREQUENCY			433E6  // 433 MHz
#define TX_POWER			20     // dBm
#define BANDWIDTH			125E3  // 125 kHz
#define SYNC_WORD			0x12
#define SPREAD_FACTOR		7
#define CODING_RATE			5
#define PREAMBLE			8
#define LORA_WAKE_TIMEOUT	8000

// CSMA/CA Settings
#define SEND_ATTEMPTS		2
#define BACKOFF_MUL			75
#define CSMA_NOISE_LIM		-90
#define CSMA_TOUT_MUL		500
#define CSMA_TOUT_MIN		3000

// Algorithm Settings
#define START_OF_BRACKET	5
#define DECIMAL_VALUES  	5
#define MAX_MESSAGE_LENGTH  86
#define EPSILON         	0.0001
#define BLANK_PLACEHOLDER	'*'

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
			TIME,
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
			"TIME:",
			"DATE:"
		}; // TODO: Code to sync time and date

		uint16_t _backoffTime;
		uint16_t _csmaTimeout;
		bool _newpayloadAlert;
		uint8_t _hwid;
		uint8_t _sendAttempts;
		unsigned long _lastSystemUpdateTime;
		float _sensorData[VALID_HEADERS];
		float _systemValues[MAX_DEVICES];
		String _loraPayload;
		String _loraprevHeader;
		
		void resetValues();
		bool getLoRaPayload();
		bool checkMessageValidity();
		void preloadMessageData();
		void processPayloadData();
		void sendPayloadData();

		#ifdef ENCRYPTING
			void rc4EncryptDecrypt(char *data, uint8_t len);
		#endif


	public:
		void Initialize(IDATA IData);
		void loadSensorData(IDATA IData);
		void startLoRaMesh(IDATA IData);
};

#endif
