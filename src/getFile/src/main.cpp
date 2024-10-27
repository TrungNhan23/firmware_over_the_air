#include "FlashSTM32.h"

const char *ssid = "Nhennn";
const char *pass = "trungnhan0203";

const char *url = "http://192.168.0.123:2400/uploads/led.hex";

HardwareSerial FlashPort(2);
FlashSTM32 devKit(18, 14);

void setup()
{
    Serial.begin(115200);
    FlashPort.begin(9600, SERIAL_8N1, 16, 17);
    devKit.setup();

    WiFi.begin(ssid, pass);
    Serial.write("Connecting to wifi ");
    Serial.write(ssid);
    Serial.write(":");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.write(".");
        delay(100);
    }
    Serial.println();
    Serial.print("Connected to wifi ");
    Serial.write(ssid);
    Serial.println();

    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount file system");
        return;
    }
    else
    {
        Serial.println("Successful to mount file system");
    }

    if (devKit.DownloadFirmware(url))
    {
        Serial.println("Download hex file successfully!!!");
        File firmwareFile = SPIFFS.open("/led.hex", "r");
        if (!firmwareFile)
        {
            Serial.println("Failed to open firmware file");
            return;
        }

        size_t fileSize = firmwareFile.size();
        Serial.print("Firmware file size: ");
        Serial.println(fileSize);

        // Attempt to enter boot mode before flashing
        if (!devKit.enterBootMode(FlashPort))
        {
            Serial.println("Failed to enter bootloader mode.");
            return; // Stop if we can't enter bootloader mode
        }

        // Proceed with flashing
        devKit.Flash(firmwareFile, FlashPort);
        devKit.exitBootMode();
        Serial.println("Firmware upload complete");
    }
    else
    {
        Serial.println("Firmware download failed");
    }
}
void loop()
{
}
