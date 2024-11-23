#include "FlashSTM32.h"

int bini = 0, rdtmp;

uint8_t hexCharToByte(const char *hex)
{
    uint8_t value = 0;
    sscanf(hex, "%2hhx", &value);
    return value;
}

int parseHexLine(const char *line, IntelHexFile &record)
{
    if (line[0] != ':')
    {
        return -1;
    }

    // Parse byteCount
    record.byteCount = hexCharToByte(&line[1]);

    // Parse address (2 byte)
    record.address = (hexCharToByte(&line[3]) << 8) | hexCharToByte(&line[5]);

    // Parse recordType
    record.recordType = hexCharToByte(&line[7]);

    // Parse data
    for (int i = 0; i < record.byteCount; i++)
    {
        record.data[i] = hexCharToByte(&line[9 + i * 2]);
    }

    // Parse checksum
    record.checksum = hexCharToByte(&line[9 + record.byteCount * 2]);

    return 0;
}

int parseHexFile(File &file, IntelHexFile &record, FlashSTM32 &STM32, HardwareSerial &flashPort)
{
    if (!file)
    {
        Serial.println("Failed to open file");
        return -1;
    }

    char line[100];
    int countLine = 0;
    while (file.available())
    {
        file.readBytesUntil('\n', line, sizeof(line));
        IntelHexFile record;
        Serial.println("Atempting to flash.");
        if (parseHexLine(line, record) == 0)
        {
            Serial.print("Flash line: ");
            Serial.println(countLine);
            countLine++;
        }
        // Serial.println("Parsed successfully:");
        // Serial.print("Byte count: ");
        // Serial.println(record.byteCount);
        // Serial.print("Address: 0x");
        // Serial.println(record.address, HEX);
        // Serial.print("Record type: ");
        // Serial.println(record.recordType);
        // Serial.print("Data: ");
        // for (int i = 0; i < record.byteCount; i++)
        // {
        //     Serial.print(record.data[i], HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();
        // Serial.print("Checksum: 0x");
        // Serial.println(record.checksum, HEX);
    }

    file.close();
    return -1;
}

String FindNameOfFile(String url)
{
    size_t lastSlashIndex = url.lastIndexOf('/');
    return url.substring(lastSlashIndex + 1);
}

uint8_t Data2Checksum(const String &line)
{
    if (line[0] != ':')
    {
        Serial.println("HEX file line: Missing start code ':'");
        return 0;
    }

    uint8_t checksum = 0;

    for (size_t i = 1; i < line.length() - 2; i += 2)
    {
        String byteString = line.substring(i, i + 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
        checksum += byte;
    }

    checksum = (~checksum + 1) & 0xFF; // calc the 2's complements

    return checksum;
}

void FlashSTM32::sendCMD(uint8_t cmd, HardwareSerial &flashPort)
{
    flashPort.write(cmd);
    flashPort.write(~cmd);
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
        File file = SPIFFS.open("/blink.hex", "w");
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
    this->sendCMD(EraseCMD, flashPort);

    uint8_t byte = flashPort.read();
    Serial.print("Received byte: 0x");
    Serial.println(byte, HEX);
    Serial.println("ACK received, continue to erase code.");
    this->sendCMD(Erase_extend_CMD, flashPort);
    Serial.println("ACK received, Erase command accepted.");
}

void FlashSTM32::Flash(File &firmwareFile, HardwareSerial &flashPort)
{
    size_t fileSize = firmwareFile.size();
    uint32_t bytesSent = 0;
    int lastbuf = 0;
    uint8_t binData[256];
    IntelHexFile record;
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
    else
    {
        this->Erase(flashPort);
        Serial.println("Flashing firmware...");
        bini = firmwareFile.size() / 256;
        lastbuf = firmwareFile.size() % 256;
        for (int i = 0; i < bini; ++i)
        {
            firmwareFile.read(binData, 256);
            this->sendCMD(WriteCMD, flashPort);
        }
    }

    // while (firmwareFile.available())
    // {
    //     if (parseHexFile(firmwareFile, record, *this, flashPort) != 0)
    //         Serial.println("Failed to flash firmware file.");
    // }
    // Serial.println("Flashing completed.");
}

FlashSTM32::~FlashSTM32()
{
}
