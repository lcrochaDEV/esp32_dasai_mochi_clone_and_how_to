#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>

class Animations {
  public:
    Animations();
    void helloWordMochi();
    void animationsLoop();
    void animeScreen(String consoleText);
    void not_wifi();

  private:
    String animations;
    void setFrameData(const unsigned char* frameData[]);
};

#endif