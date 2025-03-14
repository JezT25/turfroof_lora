#include <SPI.h>
#include <LoRa.h>

#define LORA_NSS        4  // Chip select (NSS)
#define LORA_RST        3  // Reset pin
#define LORA_DI0        2  // Interrupt pin (DIO0)

#define FREQUENCY       433E6   // 433 MHz
#define TX_POWER        17      // dBm
#define BANDWIDTH       125E3   // 125 kHz
#define SPREAD_FACTOR   7       // SF7 (Valid range: 6-12)
#define CODING_RATE     5       // 4/5
#define SYNC_WORD       0x12    // Private LoRa sync word
#define PREAMBLE_LENGTH 8       // Preamble symbols

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Initializing LoRa Transmitter...");

  // Set LoRa module pins
  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DI0);

  // Start LoRa communication
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("LoRa init failed! Check connections.");
    while (1);  // Halt execution
  }

  LoRa.setSpreadingFactor(SPREAD_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setPreambleLength(PREAMBLE_LENGTH);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setTxPower(TX_POWER);

  Serial.println("LoRa Transmitter Initialized at 433 MHz!");
}

String message = "TEMP:[]";  // Message to send

void loop() {
  Serial.print("Sending message: ");
  Serial.println(message);

  // Send message
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println("Message sent successfully!");

  // Wait for a reply
  Serial.println("Waiting for reply...");
  unsigned long startTime = millis();
  bool receivedReply = false;

  while (1) {  // Wait up to 5 seconds for a reply
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      Serial.println("Reply received!");

      String reply = "";
      while (LoRa.available()) {
        reply += (char)LoRa.read();
      }

      Serial.print("Reply: ");
      Serial.println(reply);
      receivedReply = true;
      message = "TEMP:[0.000,0.000,2.222,0.000,0.000,0.000,0.000,0.000]";
      break;  // Exit the loop once a reply is received
    }
  }
}
