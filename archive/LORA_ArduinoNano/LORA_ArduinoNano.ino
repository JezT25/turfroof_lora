#include <SPI.h>
#include <LoRa.h>

#define FREQUENCY 433E6
#define TX_POWER 20
#define BANDWIDTH 125E3
#define SYNC_WORD 0x12
#define SPREAD_FACTOR 12
#define CODING_RATE 5
#define PREAMBLE 8

#define LORA_SS 4
#define LORA_RST 3
#define LORA_DIO0 2

#define RC4_BYTES 255
#define ENCRYPTION_KEY "G7v!Xz@a?>Qp!d$1"

void rc4EncryptDecrypt(char *data, int len)
{
    int S[RC4_BYTES];
    for (int i = 0; i < RC4_BYTES; i++)
    {
        S[i] = i;
    }

    int j = 0, temp;
    int enc_len = strlen(ENCRYPTION_KEY);
    for (int i = 0; i < RC4_BYTES; i++)
    {
        j = (j + S[i] + ENCRYPTION_KEY[i % enc_len]) % RC4_BYTES;
        temp = S[i];
        S[i] = S[j];
        S[j] = temp;
    }

    int rnd = 0, i = 0;
    j = 0;
    for (int n = 0; n < len; n++)
    {
        i = (i + 1) % RC4_BYTES;
        j = (j + S[i]) % RC4_BYTES;
        temp = S[i];
        S[i] = S[j];
        S[j] = temp;
        rnd = S[(S[i] + S[j]) % RC4_BYTES];
        data[n] ^= rnd; // XOR for encryption/decryption
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("LoRa Encrypted Send/Receive");

    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(FREQUENCY))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }

    LoRa.setTxPower(TX_POWER);
    LoRa.setSpreadingFactor(SPREAD_FACTOR);
    LoRa.setSignalBandwidth(BANDWIDTH);
    LoRa.setCodingRate4(CODING_RATE);
    LoRa.setSyncWord(SYNC_WORD);
    LoRa.setPreambleLength(PREAMBLE);

    Serial.println("LoRa init succeeded.");
    Serial.println("Type message to send:");
}

void printHex(char *data, int length)
{
    for (int i = 0; i < length; i++)
    {
        if ((uint8_t)data[i] < 16)
            Serial.print("0");
        Serial.print((uint8_t)data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void loop()
{
    // Receive LoRa packets
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        Serial.println("\nReceived:");

        char encrypted[packetSize];
        int i = 0;
        while (LoRa.available() && i < packetSize)
        {
            encrypted[i++] = (char)LoRa.read();
        }

        Serial.print("Encrypted: ");
        printHex(encrypted, i);

        rc4EncryptDecrypt(encrypted, i);
        encrypted[i] = '\0'; // Null-terminate after decrypting

        Serial.print("Decrypted: ");
        Serial.println(encrypted);

        int rssi = LoRa.packetRssi();
        Serial.print("RSSI: ");
        Serial.println(rssi);
    }

    // Send messages entered on Serial
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() > 0)
        {
            Serial.print("\nSent: ");
            Serial.println(input);

            int len = input.length();
            char buf[len + 1];
            input.toCharArray(buf, len + 1);

            rc4EncryptDecrypt(buf, len);

            Serial.print("Encrypted: ");
            printHex(buf, len);

            LoRa.beginPacket();
            LoRa.write((uint8_t *)buf, len);
            LoRa.endPacket();
        }
    }
}
