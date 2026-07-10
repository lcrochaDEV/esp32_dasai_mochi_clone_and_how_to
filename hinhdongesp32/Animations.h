#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>


class Animations {
  public:
    Animations();
    void helloWordMochi(String consoleText = "Hello Mochi");
    void animationsLoop();
    void not_wifi();
    void control_oled_power(bool enable);
    
    void processHexFrameLoop(); 
    void drawHexFrame(const char* hexData);
    
  private:
    void setFrameData(const unsigned char* frameData[]);
    String animations;

    String _currentHexData = "";      // Guarda a última string de frame recebida
    bool _newFrameAvailable = false;  // Flag para saber se há frame novo para desenhar
    unsigned long _lastHexFrameTime = 0; // Controle do tempo via millis

};

#endif