#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

class Animations {
  public:
    Animations();
    void helloWordMochi(String consoleText = "Hello Mochi");
    void animationsLoop();
    void not_wifi();
    void control_oled_power(bool enable);

  private:
    void setFrameData(const unsigned char* frameData[]);
    String animations;
};

#endif