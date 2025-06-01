#include <SPI.h>
#include <LoRa.h>

#define LORA_NSS   D8   // Chip Select (NSS)

// Define SPI pins for Wemos D1 Mini
#define SCK   D5   // GPIO14
#define MISO  D6   // GPIO12
#define MOSI  D7   // GPIO13

#define FREQUENCY       433E6  // 433 MHz
#define TX_POWER        20     // dBm
#define BANDWIDTH       125E3  // 125 kHz
#define SYNC_WORD       0x12
#define SPREAD_FACTOR   7
#define CODING_RATE     5
#define PREAMBLE        8

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Initializing LoRa Transmitter...");

  // Initialize SPI (without arguments for ESP8266)
  SPI.begin();

  // Set LoRa module pins (Only NSS is needed)
  LoRa.setPins(LORA_NSS, D1);

  // Start LoRa communication
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("X: LoRa init failed! Check connections.");
    while (1);  // Halt execution
  }

  LoRa.setSpreadingFactor(SPREAD_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setPreambleLength(PREAMBLE);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setTxPower(TX_POWER);

  // Enable CRC
	LoRa.enableCrc();

  String message = "TEMP:[]";  // Message to send
  // String message = "HUMI:[]";  // Message to send
  // String message = "STMP:[]";  // Message to send
  // String message = "SMOI:[]";  // Message to send
  // String message = "BATT:[]";  // Message to send
  // String message = "DATE:[10,11,12,13,01,25]";  // Message to send

  Serial.println("LoRa Transmitter Initialized at 433 MHz!");
  Serial.print("Sending message: ");
  Serial.println(message);
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket(true);
  Serial.println("Message sent successfully!");

  Serial.println("Waiting for reply...");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println("Reply received!");

    String reply = "";
    while (LoRa.available()) {
      reply += (char)LoRa.read();
    }

    Serial.print("Reply: ");
    Serial.println(reply);
  }
}
