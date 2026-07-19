#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>


class Animations {
  public:
    Animations();
    void helloWordMochi(String consoleText = "Hello Mochi");
    void animationsLoop();
    void not_wifi(); //Amimation
    void noworker(); //Amimation
    void bugframe(); //Amimation
    
    void processHexFrameLoop(); 
    void drawHexFrame(const char* hexData);

    /**
     * @brief Retorna se o display OLED está ligado no momento.
     * @return true se ligado, false se desligado.
     */
    bool is_oled_on() const { return _oled_active; }
    void control_oled_power(bool enable);
    
  private:
    void setFrameData(const unsigned char* frameData[]);
    String animations;

    char _currentHexData[2050];     // Guarda a última string de frame recebida
    bool _newFrameAvailable = false;  // Flag para saber se há frame novo para desenhar
    unsigned long _lastHexFrameTime = 0; // Controle do tempo via millis
    bool _oled_active = true;

};

#endif