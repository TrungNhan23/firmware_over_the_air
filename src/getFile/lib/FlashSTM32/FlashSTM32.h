#include <HardwareSerial.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <HTTPClient.h>


#define ACK_MESSAGE 0x79
#define NAK_MESSAGE 0x1F

String FindNameOfFile(String url);


class FlashSTM32
{
private:
    uint8_t port;
    int NRST_PIN;
    int BOOT0_PIN;

public:
    FlashSTM32(int rst_pin, int boot0_pin);
    void setup();
    bool DownloadFirmware(String url);
    bool parseIntelHexLine(String line, uint8_t* data, uint8_t* length, uint32_t* address);  
    bool SendFirmware(uint8_t *data, size_t length, uint32_t address, HardwareSerial &flashPort);
    bool enterBootMode(HardwareSerial &flashPort);
    void exitBootMode();
    void Flash(File &firmwareFile, HardwareSerial &stm32);
    void Erase();
    ~FlashSTM32();
};
