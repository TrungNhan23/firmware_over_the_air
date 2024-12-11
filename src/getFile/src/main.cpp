// #include "FlashSTM32.h"

// const char *ssid = "Nhennn";
// const char *pass = "trungnhan0203";

// const char *url = "http://192.168.0.123:2400/uploads/testBlink.hex";

// HardwareSerial FlashPort(2);
// FlashSTM32 devKit(18, 14);

// void setup()
// {
//     Serial.begin(115200);
//     FlashPort.begin(115200, SERIAL_8E1, 16, 17);
//     delay(500);

//     devKit.setup();

//     WiFi.begin(ssid, pass);
//     Serial.write("Connecting to wifi ");
//     Serial.write(ssid);
//     Serial.write(":");
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.write(".");
//         delay(100);
//     }
//     Serial.println();
//     Serial.print("Connected to wifi ");
//     Serial.write(ssid);
//     Serial.println();

//     if (!SPIFFS.begin(true))
//     {
//         Serial.println("Failed to mount file system");
//         return;
//     }
//     else
//     {
//         Serial.println("Successful to mount file system");
//     }

//     if (devKit.DownloadFirmware(url))
//     {
//         Serial.println("Download hex file successfully!!!");
//         File firmwareFile = SPIFFS.open("/testBlink.hex", "r");
//         if (!firmwareFile)
//         {
//             Serial.println("Failed to open firmware file");
//             return;
//         }

//         size_t fileSize = firmwareFile.size();
//         Serial.print("Firmware file size: ");
//         Serial.println(fileSize);

//         if (!devKit.enterBootMode(FlashPort))
//         {
//             Serial.println("Failed to enter bootloader mode.");
//             return;
//         }

//         devKit.Flash(firmwareFile, FlashPort);
//         delay(5);
//         devKit.exitBootMode();
//         Serial.println("Firmware upload complete");

//     }
// }
// void loop()
// {
// }

#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>

// WiFi credentials
const char *ssid = "Nhennn";
const char *pass = "trungnhan0203";
void setup() {
  // Khởi tạo cổng serial để theo dõi
  Serial.begin(115200);
  
  // Kết nối đến mạng WiFi
  WiFi.begin(ssid, pass);
  
  // Chờ kết nối WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");
}

void loop() {
  // Kiểm tra kết nối WiFi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Tạo URL cho API
    String url = "http://192.168.x.x/data";  // Thay đổi với URL của bạn
    
    // Bắt đầu HTTP request
    http.begin(url);
    
    // Gửi yêu cầu GET
    int httpCode = http.GET();

    // Nếu mã phản hồi là 200 (OK), đọc và in dữ liệu
    if (httpCode > 0) {
      String payload = http.getString();  // Lấy nội dung JSON trả về từ server
      Serial.println("Response from server:");
      Serial.println(payload);
      
      // Nếu dữ liệu trả về là JSON, bạn có thể phân tích và xử lý tại đây.
    } else {
      Serial.println("Error in HTTP request");
    }
    
    // Kết thúc kết nối HTTP
    http.end();
  } else {
    Serial.println("Disconnected from WiFi");
  }
  
  delay(10000);  // Chờ 10 giây trước khi yêu cầu lại
}
// // URL to download file
// const char* fileUrl = "http://192.168.0.116:8000/download/3/";

// String generateUniqueFileName() {
//   static int fileCounter = 0;
//   fileCounter++;
//   return "downloaded_file_" + String(fileCounter) + ".hex";
// }

// void readHexFile(String fileName) {
//   File file = SPIFFS.open("/" + fileName, FILE_READ);
//   if (!file) {
//     Serial.println("Failed to open HEX file for reading");
//     return;
//   }

//   Serial.println("Reading HEX file content:");
//   while (file.available()) {
//     String line = file.readStringUntil('\n');
//     Serial.println(line);
//   }

//   file.close();
// }

// void downloadFile(const char* url) {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     http.begin(url);

//     Serial.println("Starting file download...");

//     int httpCode = http.GET();
//     if (httpCode == HTTP_CODE_OK) {
//       String fileName = generateUniqueFileName();
//       File file = SPIFFS.open("/" + fileName, FILE_WRITE);
//       if (!file) {
//         Serial.println("Failed to open file for writing");
//         return;
//       }

//       // Write the data to file
//       file.write((uint8_t *)http.getString().c_str(), http.getSize());
//       file.close();

//       Serial.println("File downloaded: " + fileName);

//       // Read and print HEX file content
//       readHexFile(fileName);
//     } else {
//       Serial.printf("Failed to download file, HTTP code: %d\n", httpCode);
//     }

//     http.end();
//   } else {
//     Serial.println("WiFi not connected");
//   }
// }

// void setup() {
//   Serial.begin(115200);

//   // Initialize SPIFFS
//   if (!SPIFFS.begin(true)) {
//     Serial.println("Failed to mount SPIFFS");
//     return;
//   }

//   // Connect to WiFi
//   WiFi.begin(ssid, pass);
//   Serial.print("Connecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.print(".");
//   }
//   Serial.println("\nConnected to WiFi");

//   // Download the file
//   downloadFile(fileUrl);
// }

// void loop() {
//   // Nothing to do here
// }