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

#define RC4_BYTES 		255
#define ENCRYPTION_KEY  "G7v!Xz@a?>Qp!d$1"

void rc4EncryptDecrypt(char *data, int len) {
    int S[RC4_BYTES];
    for (int i = 0; i < RC4_BYTES; i++) {
        S[i] = i;
    }

    int j = 0, temp;
    int enc_len = strlen(ENCRYPTION_KEY);
    for (int i = 0; i < RC4_BYTES; i++) {
        j = (j + S[i] + ENCRYPTION_KEY[i % enc_len]) % RC4_BYTES;
        temp = S[i];
        S[i] = S[j];
        S[j] = temp;
    }

    int rnd = 0, i = 0; j = 0;
    for (int n = 0; n < len; n++) {
        i = (i + 1) % RC4_BYTES;
        j = (j + S[i]) % RC4_BYTES;
        temp = S[i];
        S[i] = S[j];
        S[j] = temp;
        rnd = S[(S[i] + S[j]) % RC4_BYTES];
        data[n] ^= rnd;  // XOR for encryption/decryption
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Initializing LoRa Transmitter...");

    SPI.begin();
    LoRa.setPins(LORA_NSS, D1);

    if (!LoRa.begin(FREQUENCY)) {
        Serial.println("X: LoRa init failed! Check connections.");
        while (1);
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
    int msgLen = message.length();

    char buffer[msgLen + 1];
    message.toCharArray(buffer, msgLen + 1);  // +1 for null terminator
    rc4EncryptDecrypt(buffer, msgLen);  // Encrypt

    Serial.println("LoRa Transmitter Initialized at 433 MHz!");
    Serial.print("Sending Encrypted Message: ");
    for (int i = 0; i < msgLen; i++) {
        Serial.print(message[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    LoRa.beginPacket();
    LoRa.write((uint8_t*)buffer, msgLen);
    LoRa.endPacket(true);
    
    Serial.println("Message sent successfully!");
    Serial.println("Waiting for reply...");
}

void loop() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.println("Reply received!");

        char reply[packetSize + 1];
        int i = 0;
        while (LoRa.available()) {
            reply[i++] = (char)LoRa.read();
        }
        reply[i] = '\0';  // Null-terminate the string

        Serial.print("Encrypted Reply: ");
        for (int j = 0; j < i; j++) {
            Serial.print(reply[j], HEX);
            Serial.print(" ");
        }
        Serial.println();

        // Decrypt message
        rc4EncryptDecrypt(reply, i);
        Serial.print("Decrypted Reply: ");
        Serial.println(reply);
    }
}
