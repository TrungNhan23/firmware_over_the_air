#include "FlashSTM32.h"

String FindNameOfFile(String url)
{
    size_t lastSlashIndex = url.lastIndexOf('/');
    return url.substring(lastSlashIndex + 1);
}

// String FindNameOfFile(co url){
//     size_t lastSlashIndex = url.lastIndexOf('/');
//     return url.substring(lastSlashIndex + 1);
// }

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

bool FlashSTM32::parseIntelHexLine(String line, uint8_t *data, uint8_t *length, uint32_t *address)
{
    if (line[0] != ':')
    {
        Serial.println("Invalid HEX line: Missing starting ':'");
        return false;
    }

    // Chuyển đổi số byte
    int byteCount = strtol(line.substring(1, 3).c_str(), NULL, 16);
    if (byteCount <= 0 || byteCount > 255)
    {
        Serial.println("Invalid HEX line: Incorrect byte count");
        return false;
    }

    // Chuyển đổi địa chỉ (4 kí tự tiếp theo là địa chỉ 16-bit)
    *address = strtol(line.substring(3, 7).c_str(), NULL, 16);
    if (*address > 0xFFFF)
    {
        Serial.println("Invalid HEX line: Incorrect address");
        return false;
    }

    // Chuyển đổi loại bản ghi (sau 4 ký tự địa chỉ)
    int recordType = strtol(line.substring(7, 9).c_str(), NULL, 16);
    if (recordType != 0x00)
    { // 0x00: dữ liệu thực tế, các loại khác không dùng trong upload firmware
        Serial.println("Non-data record type, skipping line");
        return false;
    }

    *length = byteCount;

    // Chuyển đổi các byte dữ liệu
    for (int i = 0; i < byteCount; i++)
    {
        data[i] = strtol(line.substring(9 + i * 2, 11 + i * 2).c_str(), NULL, 16);
        if (data[i] > 0xFF)
        {
            Serial.printf("Invalid data byte at position %d\n", i);
            return false;
        }
    }

    Serial.println("Parsed HEX line successfully");
    return true;
}

bool FlashSTM32::enterBootMode(HardwareSerial &flashPort)
{
    // Thiết lập để vào chế độ bootloader
    digitalWrite(this->BOOT0_PIN, 1);
    delay(10);
    digitalWrite(this->NRST_PIN, 0);
    delay(100);
    digitalWrite(this->NRST_PIN, 1);

    delay(100); // Đợi một khoảng thời gian dài hơn để STM32 chuẩn bị

    // Gửi lệnh khởi tạo
    flashPort.write((uint8_t)0x7F);

    delay(500); // Đợi STM32 phản hồi
    Serial.println("Attempting to enter bootloader mode...");

    while (flashPort.available())
    {
        uint8_t byte = flashPort.read();
        Serial.print("Received byte: 0x");
        Serial.println(byte, HEX);
        if (byte == ACK_MESSAGE)
        { // Kiểm tra xem byte có bằng ACK không
            Serial.println("ACK received, STM32 is in bootloader mode.");
            return true;
        }
        delay(50); // Tăng độ trễ giữa các lần đọc byte
    }

    Serial.println("No response from STM32.");
    return false;
}

void FlashSTM32::exitBootMode()
{
    digitalWrite(this->BOOT0_PIN, 0);
    delay(1);
    digitalWrite(this->NRST_PIN, 0);
    delay(10);
    digitalWrite(this->NRST_PIN, 1);
}

bool FlashSTM32::SendFirmware(uint8_t *data, size_t length, uint32_t address, HardwareSerial &flashPort)
{
    flashPort.write(0x31);
    flashPort.write(0xCE);
    delay(10);

    if (flashPort.read() != ACK_MESSAGE)
        return false;

    uint8_t start_add[5] = {
        (uint8_t)(address >> 24),
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)(address),
        (uint8_t)((address >> 24) ^ (address >> 16) ^ (address >> 8) ^ (address)) // Checksum
    };
    // uint8_t start_add[5] = {0x08, 0x00, 0x00, 0x00, 0xF7};
    // start add is 0x08000000 and checksum = 0x08 ^ 0x00 ^ 0x00 ^ 0x00 ^ 0x00= 0xf7
    flashPort.write(start_add, 5);

    if (flashPort.read() != ACK_MESSAGE)
        return false;

    flashPort.write(length - 1);
    flashPort.write(data, length);

    uint8_t checksum = length - 1;
    for (int i = 0; i < length; ++i)
    {
        checksum ^= data[i];
    }

    flashPort.write(checksum);
    return (flashPort.read() == ACK_MESSAGE);
}

void FlashSTM32::Flash(File &firmwareFile, HardwareSerial &stm32)
{
    uint8_t ack;
    size_t fileSize = firmwareFile.size();
    size_t bytesSent = 0;

    if (fileSize == 0)
    {
        Serial.println("Firmware file is empty.");
        return;
    }

    // Ensure the file is readable
    if (!firmwareFile)
    {
        Serial.println("Failed to open firmware file.");
        return;
    }

    // // Check if STM32 is in bootloader mode
    // Serial.println("Attempting to enter bootloader mode...");
    // stm32.write(0x7F);  // Send initial command to enter bootloader mode

    // if (stm32.readBytes(&ack, 1) != 1 || ack != 0x79) {
    //     Serial.println("Failed to enter bootloader mode");
    //     return;
    // }
    // Serial.println("ACK received, STM32 is in bootloader mode.");

    // Erase flash memory before flashing
    Serial.println("Sending Erase command...");
    stm32.write(0x43);          // Command code for 'Erase'
    uint8_t eraseSector = 0x00; // Adjust this value based on the sector you want to erase
    stm32.write(eraseSector);   // Send the sector to erase
    stm32.write(0xBC);          // Checksum for the erase command

    // Wait for ACK response
    if (stm32.readBytes(&ack, 1) != 1)
    {
        Serial.println("No response received after erase command.");
        return;
    }
    Serial.print("Received byte after erase command: 0x");
    Serial.println(ack, HEX);

    if (ack != 0x79)
    {
        Serial.println("Erase command failed.");
        return;
    }
    Serial.println("ACK received, Erase command accepted.");

    // Start writing firmware
     Serial.println("Flashing firmware...");

    while (firmwareFile.available()) {
        uint8_t buffer[256];
        int bytesRead = firmwareFile.read(buffer, sizeof(buffer));
        
        if (bytesRead <= 0) {
            Serial.println("Failed to read from firmware file.");
            return;
        }


///fix here
        // Send Write Memory command
        stm32.write(0x31); // Command to write memory
        if (stm32.readBytes(&ack, 1) != 1 || ack != 0x79) {
            Serial.println("Write Memory command failed.");
            return;
        }

        // Calculate target address
        uint32_t address = 0x08000000 + bytesSent; // Start address for STM32 flash
        uint8_t addr[5] = {
            (uint8_t)((address >> 24) & 0xFF),
            (uint8_t)((address >> 16) & 0xFF),
            (uint8_t)((address >> 8) & 0xFF),
            (uint8_t)(address & 0xFF),
            (uint8_t)((address >> 24) ^ (address >> 16) ^ (address >> 8) ^ address) // XOR checksum
        };

        stm32.write(addr, 5); // Send address
        if (stm32.readBytes(&ack, 1) != 1 || ack != 0x79) {
            Serial.println("Address command failed.");
            return;
        }

        // Send data length
        stm32.write(bytesRead - 1); // Length is bytesRead - 1
        uint8_t checksum = bytesRead - 1;

        // Send the data
        for (int i = 0; i < bytesRead; i++) {
            stm32.write(buffer[i]);
            checksum ^= buffer[i]; // Update checksum
        }
        stm32.write(checksum); // Send the checksum

        // Wait for acknowledgment
        if (stm32.readBytes(&ack, 1) != 1 || ack != 0x79) {
            Serial.println("Data write failed.");
            return;
        }

        bytesSent += bytesRead;
        Serial.print("Sent: ");
        Serial.print(bytesSent);
        Serial.print("/");
        Serial.println(fileSize);
    }

    Serial.println("Flashing complete.");
}

FlashSTM32::~FlashSTM32()
{
}
