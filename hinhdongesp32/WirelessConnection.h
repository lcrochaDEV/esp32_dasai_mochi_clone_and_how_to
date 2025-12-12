#ifndef WIRELESSCONNECTION_H
#define WIRELESSCONNECTION_H


#include <Arduino.h>

#include <WiFi.h>

#include "WifiConnect.h"
#include "BTConnect.h"
#include "Animations.h"

class WirelessConnection: public WifiConnect, public BTConnect{
  public:
      WirelessConnection (const char* ssid = nullptr, const char* password = nullptr, Animations* animationPtr = nullptr);
};
 
#endif