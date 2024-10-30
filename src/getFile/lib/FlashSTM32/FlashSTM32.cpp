#include "FlashSTM32.h"

String FindNameOfFile(String url)
{
    size_t lastSlashIndex = url.lastIndexOf('/');
    return url.substring(lastSlashIndex + 1);
}

FlashSTM32::FlashSTM32(int rst_pin, int boot0_pin)
{
    this->BOOT0_PIN = boot0_pin;
    this->NRST_PIN = rst_pin;
}

void FlashSTM32::setup()
{
    pinMode(BOOT0_PIN, OUTPUT);
    pinMode(NRST_PIN, OUTPUT);
}

bool FlashSTM32::DownloadFirmware(String url)
{
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        File file = SPIFFS.open("/led.hex", "w");
        if (!file)
        {
            Serial.println("Failed to open file for writing");
            return false;
        }
        String payload = http.getString();
        file.print(payload);
        file.close();
        Serial.println("Firmware downloaded and saved to SPIFFS");
        return true;
    }
    else
    {
        Serial.printf("Failed to download firmware, error: %d\n", httpCode);
        return false;
    }
}


bool FlashSTM32::enterBootMode(HardwareSerial &flashPort)
{

    digitalWrite(this->BOOT0_PIN, 1);
    delay(100);
    digitalWrite(this->NRST_PIN, 0);
    delay(100);
    digitalWrite(this->NRST_PIN, 1);

    delay(100);

    flashPort.write((uint8_t)0x7F);

    delay(500);
    Serial.println("Attempting to enter bootloader mode...");

    // while (flashPort.available())
    // {
    //     uint8_t byte = flashPort.read();
    //     Serial.print("Received byte: 0x");
    //     Serial.println(byte, HEX);
    //     if (byte == ACK_MESSAGE)
    //     {
    //         Serial.println("ACK received, flashPort is in bootloader mode.");
    //         return true;
    //     }
    //     delay(50);
    // }

    uint8_t byte = flashPort.read();
    Serial.print("Received byte: 0x");
    Serial.println(byte, HEX);
    if (byte == ACK_MESSAGE)
    {
        Serial.println("ACK received, flashPort is in bootloader mode.");
        return true;
    }

    Serial.println("No response from flashPort.");
    return false;
}

void FlashSTM32::exitBootMode()
{
    Serial.println("Exiting bootloader mode...");
    digitalWrite(this->BOOT0_PIN, 0);
    delay(100);
    digitalWrite(this->NRST_PIN, 0);
    delay(100);
    digitalWrite(this->NRST_PIN, 1);
    delay(100);
    Serial.println("Exit bootloader mode done");
}

void FlashSTM32::Erase(HardwareSerial &flashPort)
{

    Serial.println("Attempting to Erase code...");

    flashPort.write((uint8_t)0x43);
    delay(5);
    flashPort.write((uint8_t)0xBC);
    delay(5);

    uint8_t byte = flashPort.read();
    Serial.print("Received byte: 0x");
    Serial.println(byte, HEX);
    if (byte == ACK_MESSAGE)
    {
        Serial.println("ACK received, continue to erase code.");
        flashPort.write((uint8_t)0xFF);
        delay(5);
        flashPort.write((uint8_t)0x00);
        delay(5);

        Serial.println("ACK received, Erase command accepted.");
    }
}

void FlashSTM32::Flash(File &firmwareFile, HardwareSerial &flashPort)
{
    uint8_t ack;
    size_t fileSize = firmwareFile.size();
    uint8_t bytesSent = 0;

    if (fileSize == 0)
    {
        Serial.println("Firmware file is empty.");
        return;
    }

    if (!firmwareFile)
    {
        Serial.println("Failed to open firmware file.");
        return;
    }

    this->Erase(flashPort);

    Serial.println("Flashing firmware...");

    while (firmwareFile.available())
    {
        // uint8_t buffer[firmwareFile.size()];
        uint8_t buffer[128];
        int bytesRead = firmwareFile.read(buffer, sizeof(buffer));

        if (bytesRead <= 0)
        {
            Serial.println("Failed to read from firmware file.");
            return;
        }

        flashPort.write((uint8_t)0x31);
        delay(5);
        flashPort.write((uint8_t)0xCE);
        delay(10);

        uint8_t byte = flashPort.read();
        Serial.print("Received byte: 0x");
        Serial.println(byte, HEX);
        if (byte == ACK_MESSAGE)
        {
            uint32_t address = 0x08000000 + bytesSent;
            uint8_t addr[5] = {
                (uint8_t)(address >> 24),
                (uint8_t)(address >> 16),
                (uint8_t)(address >> 8),
                (uint8_t)(address),
                (uint8_t)((address >> 24) ^ (address >> 16) ^ (address >> 8) ^ address)};

            flashPort.write(addr, 5);
            delay(5);

            byte = flashPort.read();
            Serial.print("Received byte: 0x");
            Serial.println(byte, HEX);
            if (byte == ACK_MESSAGE)
            {
                flashPort.write(bytesRead - 1);
                uint8_t checksum = bytesRead - 1;
                delay(5);

                for (int i = 0; i < bytesRead; i++)
                {
                    flashPort.write(buffer[i]);
                    checksum ^= buffer[i];
                    delay(5);
                }
                flashPort.write(checksum);
                delay(5);
                byte = flashPort.read();

                Serial.println("Verifying.....");
                Serial.print("Received byte: 0x");
                Serial.println(byte, HEX);
                if (byte != ACK_MESSAGE)
                {
                    Serial.println("Data write failed.");
                    return;
                }
            }
        }
        bytesSent += bytesRead;
        Serial.print("Sent: ");
        Serial.print(bytesSent);
        Serial.print("/");
        Serial.println(fileSize);

        //}
    }
    Serial.println("Flashing completed.");
}

FlashSTM32::~FlashSTM32()
{
}
