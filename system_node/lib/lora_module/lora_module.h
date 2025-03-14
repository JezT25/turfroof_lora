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

#define LORA_DI0        2
#define LORA_RST        3
#define LORA_NSS        4

#define FREQUENCY       433E6  // 433 MHz
#define TX_POWER        17     // dBm
#define BANDWIDTH       125E3  // 125 kHz
#define SYNC_WORD       0x12
#define SPREAD_FACTOR   7
#define CODING_RATE     5
#define PREAMBLE        8

#define BACKOFF_MIN		50
#define BACKOFF_MAX		200
#define CSMA_TIMEOUT	5000
// #define CSMA_NOISE_LIM	-90
#define CSMA_NOISE_LIM	0	//TODO FIX THIS

class LORA_MODULE_class {
    private:
		String lora_payload;
		bool new_incomingPayload = false;
    	float myData[MAX_DEVICES] = {};
		//todo figure out how to wipe data if new data is asked like temp to humi

    public:
		bool Initialize();
		void getMessages(IDATA IData, ISYSTEM ISystem);
};

#endif
