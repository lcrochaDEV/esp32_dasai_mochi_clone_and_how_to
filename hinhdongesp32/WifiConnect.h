#ifndef WIFICONNECT_H
#define WIFICONNECT_H

#include <Arduino.h>

#include <WiFi.h>

#include "Animations.h"

class WifiConnect {
  public:
    WifiConnect(const char* ssid = nullptr, const char* password = nullptr);
    void connectionsMethod();
    bool connect_status();

  private:
    const char* ssid; 
    const char* password;
};
 
#endif