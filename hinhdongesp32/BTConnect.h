#ifndef BTCONNECT_H
#define BTCONNECT_H


#include <Arduino.h>

#include <WiFi.h>
#include "Animations.h"

class BTConnect {
  public:
    BTConnect();
    void connections_bt(const char* bt_name = "MOCHI_BT");
    void requestJsonData();
  protected:
};
 
#endif