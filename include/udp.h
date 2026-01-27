#ifndef __UDP_UTILS_H
#define __UDP_UTILS_H

#include <Arduino.h>
#if defined(ESP32)
    #include <WiFi.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
#endif

#define UDP_MASTER 54321  // port used to send messages to master
#define UDP_PORT_MESSAGE 54322  // port used for outgoing messages
#define UDP_SLAVE 54323  // port used to send messages to slave

void broadcastUDPMessage(String ip, String message, uint16_t port = UDP_PORT_MESSAGE);
void broadcastUDPMessage(IPAddress ip, String message, uint16_t port = UDP_PORT_MESSAGE);
void broadcastUDPMessage(String message);
//String detectUDPRequest(WiFiUDP &listener, String header);
String detectUDPRequest(String header);
void enableLogging(bool enable);
String getUDPParam(String message, String param);
void udpListen(uint16_t port = UDP_PORT_MESSAGE);
void logMessage(String message);
void sendUDPMessage(String ip, String message, uint16_t port = UDP_PORT_MESSAGE);
void sendUDPMessage(IPAddress ip, String message, uint16_t port = UDP_PORT_MESSAGE);

#endif
