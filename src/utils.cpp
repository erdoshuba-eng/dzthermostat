#include "utils.h"

//#ifdef esp32
//#include <WiFi.h>
//#include <HTTPClient.h>
//#else

//#ifdef esp8266
//#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>
//#endif

// zyra.ro certificate
//const char* root_ca = "";

String b2s(bool b) {
  return b ? "true" : "false";
}

String leadingZero(uint8_t value) {
  if (value < 10) {
    return "0" + String(value);
  }
  return String(value);
}
/*
bool remoteLog(uint8_t level, String msg, String auth, String device) {
  if (!WiFi.isConnected()) return false;
//   Serial.println("call request");
  String content = "{\"level\":" + String(level) + ",\"data\":" + msg + "}";
  HTTPClient http;
  http.begin("https://monitor.zyra.ro/log?cmd=log", root_ca);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Basic " + auth);
  http.addHeader("X-CSRF-Token", device);
/ *  http.begin("192.168.1.28");
  http.addHeader("Content-Type", "application/json");
  String content = "";* /
  int httpResponseCode = http.POST(content);
  http.end();
  if (httpResponseCode > 0) {
    return httpResponseCode == 200;
  }
  return false;
/ *  String respHeader = "";
  String respContent = "";
  bool addToHeader = true;
  bool firstLine = true;
  bool bRet = true;
  WiFiClient client;
//   if (client.connect("192.168.1.28", 80)) {
  if (client.connect("https://monitor.zyra.ro", 433)) {
//     Serial.println("send request");
    String content = "{\"level\":" + String(level) + ",\"data\":" + msg + "}";
//     client.print(String("POST /monitor/log?cmd=log HTTP/1.1\r\n") +
//     "Host: 192.168.1.28\r\n" +
    client.print(String("POST /log?cmd=log HTTP/1.1\r\n") +
    "Host: monitor.zyra.ro\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + String(content.length()) + "\r\n" +
    "Authorization: Basic " + auth + "\r\n" +
    "X-CSRF-Token: " + device + "\r\n" +
    "Connection: close\r\n\r\n" + content);
    // read response
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (addToHeader) {
          // the content is separated from the header by an empty line
          if (line.equals("")) { addToHeader = false; }
          else {
            Serial.println(line);
            // add lines to the response header
            respHeader += line + "\n";
            if (firstLine) {
              // decode HTTP response
              int n = line.indexOf(' ');
              line.remove(0, n + 1);
              n = line.indexOf(' ');
              int responseCode = line.substring(0, n).toInt();
              bRet = responseCode == 200;
            }
            firstLine = false;
          }
        }
        else {
          if (!respContent.equals("")) { respContent += "\n"; }
          respContent += line;
        }
      }
    }
    client.stop();
  }
  Serial.println(respHeader);
  //respContent.trim();
  //Serial.println(respContent);
  //  if (respContent.equals("ok"))
  return bRet;* /
}*/
