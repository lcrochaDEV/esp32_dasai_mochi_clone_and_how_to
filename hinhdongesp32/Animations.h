#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>

class Animations {
  public:
    Animations();
    void helloWordMochi();
    void prompt_command(String consoleText);
  private:
    String animations;
};

#endif