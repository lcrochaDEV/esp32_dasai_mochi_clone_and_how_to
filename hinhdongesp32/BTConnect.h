#ifndef BTCONNECT_H
#define BTCONNECT_H


#include <Arduino.h>

#include <WiFi.h>
#include "Animations.h"


class BTConnect {
  public:
    BTConnect();
    void btbegin();
    void btClassicScan();
    void btBLEScan();

  protected:

};
 
#endif