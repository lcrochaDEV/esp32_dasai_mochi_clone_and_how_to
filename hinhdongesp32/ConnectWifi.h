#ifndef CONNECTWIFI_H
#define CONNECTWIFI_H

#include <Arduino.h>

class ConnectWifi {
  public:
    ConnectWifi(const char* ssid = nullptr, const char* password = nullptr);
    void connectionsMethod();
    bool connect_status();

  private:
    const char* ssid; 
    const char* password;
};
 
#endif