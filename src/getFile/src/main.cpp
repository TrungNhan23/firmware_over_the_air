// #include <WiFi.h>
// #include <HTTPClient.h>
// #include <WiFiClient.h>
// #include <SPIFFS.h>


// const char *ssid = "Nhennn";
// const char *pass = "trungnhan0203";

// const char *fileURL = "http://192.168.0.114:2400/uploads/blink.hex";

// void logContent(const char *url); 

// void setup()
// {
//   Serial.begin(115200);
//   delay(1000);
//   WiFi.begin(ssid, pass);
//   Serial.write("Connecting to wifi ");
//   Serial.write(ssid);
//   Serial.write(":");
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(300);
//     Serial.write(".");
//   }
//   Serial.println();
//   Serial.print("Connected to wifi ");
//   Serial.write(ssid);
//   Serial.println();

//   logContent(fileURL); 
// }

// void loop()
// {
// }

// void logContent(const char *url)
// {
//     if (WiFi.status() == WL_CONNECTED)
//     {
//         HTTPClient http;

//         // Bắt đầu kết nối và gửi yêu cầu
//         http.begin(url);
//         int httpCode = http.GET(); // Gửi yêu cầu GET

//         // Kiểm tra mã trạng thái
//         if (httpCode > 0)
//         {
//             // Nếu thành công, đọc dữ liệu
//             if (httpCode == HTTP_CODE_OK)
//             {
//                 WiFiClient *stream = http.getStreamPtr();

//                 // Đọc dữ liệu từ stream và xuất ra Serial Monitor
//                 while (http.connected() && stream->available())
//                 {
//                     uint8_t buffer[1];  // Đọc từng byte
//                     size_t size = stream->readBytes(buffer, sizeof(buffer));
                     
//                     if (size > 0)
//                     {
//                         Serial.print("Byte read: ");
//                         Serial.println(buffer[0], HEX); // Log ra byte dưới dạng hexa
//                     }
//                 }
//                 Serial.println("File downloaded successfully");
//             }
//         }
//         else
//         {
//             Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
//         }
//         http.end(); // Kết thúc kết nối
//     }
//     else
//     {
//         Serial.println("WiFi not connected");
//     }
// }


#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, 16, 17); // RX, TX for Serial1

    // Send test data
    
}

void loop() {
    Serial1.print(1);
    if (Serial1.available()) {
        uint8_t response = Serial1.read();
        Serial.println("Received: " + response);
    }

    delay(1000); 
}
