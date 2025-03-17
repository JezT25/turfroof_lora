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

#define LORA_DI0        	2
#define LORA_RST        	3
#define LORA_NSS        	4

#define FREQUENCY       	433E6  // 433 MHz
#define TX_POWER        	20     // dBm
#define BANDWIDTH       	125E3  // 125 kHz
#define SYNC_WORD       	0x12
#define SPREAD_FACTOR   	7
#define CODING_RATE     	5
#define PREAMBLE        	8

#define SEND_ATTEMPTS		1
#define BACKOFF_MUL			75
#define CSMA_NOISE_LIM		-90
#define CSMA_TOUT_MUL		500
#define CSMA_TOUT_MIN		3000

#define LORA_WAKE_TIMEOUT 	120000

class LORA_MODULE_class {
    private:
		int backoffTime;
		int csmaTimeout;
		bool new_incomingPayload;
		float systemValues[MAX_DEVICES];
		float newValues[MAX_DEVICES];
		uint8_t num_attempts;
		unsigned long lastSystemUpdateTime;
		String lora_payload;
		String lora_previousRequest;

		void resetValues();
		void getPayloadData();
		bool checkMessageValidity();
		bool preloadMessageData(IDATA IData, ISYSTEM ISystem);
		void processPayloadData(ISYSTEM ISystem);
		void sendPayloadData(ISYSTEM ISystem);

    public:
		bool Initialize(ISYSTEM ISystem);
		void getMessages(IDATA IData, ISYSTEM ISystem);
};

#endif
