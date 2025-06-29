#include <SPI.h>
#include <LoRa.h>

#define FREQUENCY     433E6     // 433 MHz
#define TX_POWER      17        // Max for PA_BOOST
#define BANDWIDTH     62.5E3    // Wider = faster, still decent range
#define SPREAD_FACTOR 9         // Faster than SF12, still long range
#define CODING_RATE   5         // 4/5, good balance
#define PREAMBLE      8         // Default, good for most cases
#define SYNC_WORD     0x12      // For private networks

#define LORA_SS 4
#define LORA_RST 3
#define LORA_DIO0 2

#define RC4_BYTES 255
#define ENCRYPTION_KEY "G7v!Xz@a?>Qp!d$1"

#define LED_PIN A5

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
    pinMode(LED_PIN, OUTPUT);

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
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        Serial.println("\n");

        char buf[128] = {0};
        int len = 0;
        while (LoRa.available() && len < (int)sizeof(buf) - 1)
        {
            buf[len++] = (char)LoRa.read();
        }
        buf[len] = '\0';

        // Decrypt the received message
        rc4EncryptDecrypt(buf, len);

        Serial.print("Received: ");
        Serial.println(buf);

        // Find the position of "ON" or "OFF" at the end
        const char *onStr = "ON]";
        const char *offStr = "OFF]";
        bool isOn = false;
        char baseMsg[100] = {0};

        if (len == 97)
        {
            isOn = false;
            strncpy(baseMsg, buf, len - 4);
            baseMsg[len - 4] = '\0';
        }
        else if (len == 96)
        {
            isOn = true;
            strncpy(baseMsg, buf, len - 3);
            baseMsg[len - 3] = '\0';
        }
        else
        {
            Serial.println("Invalid message format.");
            return;
        }

        const char *expectedBase = "TEMP:[123.45678,234.56789,345.67890,456.78901,567.89012,678.90123,789.01234,890.12345,901.234";
        if (strcmp(baseMsg, expectedBase) == 0)
        {
            Serial.print("Status: ");
            Serial.println(isOn ? "ON" : "OFF");
            Serial.print("RSSI: ");
            Serial.println(LoRa.packetRssi());
            digitalWrite(LED_PIN, isOn);
        }
        else
        {
            Serial.println("Message content mismatch.");
        }
    }
}