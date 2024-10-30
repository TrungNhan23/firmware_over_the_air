#include <HardwareSerial.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "IntelHexParser.h"

#define ACK_MESSAGE 0x79
#define NAK_MESSAGE 0x1F

String FindNameOfFile(String url);


class FlashSTM32: public IntelHexParser
{
private:
    uint8_t port;
    int NRST_PIN;
    int BOOT0_PIN;

public:
    FlashSTM32(int rst_pin, int boot0_pin);
    void setup();
    bool DownloadFirmware(String url);  
    bool enterBootMode(HardwareSerial &flashPort);
    void exitBootMode();
    void Flash(File &firmwareFile, HardwareSerial &stm32);
    void Erase(HardwareSerial &flashPort);
    ~FlashSTM32();
};
