#include <SPI.h>
#include <LoRa.h>

#define FREQUENCY     433E6     // 433 MHz
#define TX_POWER      20        // Max for PA_BOOST
#define BANDWIDTH     62.5E3    // Wider = faster, still decent range
#define SPREAD_FACTOR 10         // Faster than SF12, still long range
#define CODING_RATE   5         // 4/5, good balance
#define PREAMBLE      8         // Default, good for most cases
#define SYNC_WORD     0x12      // For private networks

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
    LoRa.enableCrc();

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
    static unsigned long lastSend = 0;
    static bool toggle = false;
    unsigned long now = millis();

    // Send message every 1 second
    if (now - lastSend >= 1000)
    {
        lastSend = now;
        // Prepare message with ON or OFF at the end
        char msg[100];
        snprintf(msg, sizeof(msg),
                 "TEMP:[123.45678,234.56789,345.67890,456.78901,567.89012,678.90123,789.01234,890.12345,901.234%s]",
                 toggle ? "ON" : "OFF");
        toggle = !toggle;

        int len = strlen(msg);
        char buf[len + 1];
        strcpy(buf, msg);

        rc4EncryptDecrypt(buf, len);

        Serial.print("\nSent: ");
        Serial.println(msg);

        Serial.print("Encrypted: ");
        printHex(buf, len);

        LoRa.beginPacket();
        LoRa.write((uint8_t *)buf, len);
        LoRa.endPacket();
    }
}
