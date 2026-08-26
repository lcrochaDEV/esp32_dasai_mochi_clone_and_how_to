#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>

class Animations {
  public:
    Animations();
    void helloWordMochi(String consoleText = "Hello Mochi");
    void defaultlocal();
    void notwifi(); 
    void noworker(); 
    void bugframe(); 
    
    void processHexFrameLoop(); 
    void drawHexFrame(const char* hexData);

    bool is_oled_on() const { return _oled_active; }
    void control_oled_power(bool enable);
    
  private:
    void setFrameData(const unsigned char* frameData[], size_t frameCount);

    char _currentHexData[2050];     
    bool _newFrameAvailable = false;  
    bool _oled_active = true;
};

#endif