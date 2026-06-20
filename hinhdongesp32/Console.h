#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>

#include "WirelessConnection.h"
#include "SDData.h"
#include "Animations.h"

class Console: public WirelessConnection, public SDData, public Animations {
  public:
    Console(const char* consoleText = nullptr);
    //MENSAGEM DE TODO O PROGRAMA
    void helloWord(const char* consoleText = "Hello Mochi");
    void menssageViewMsg(const char* consoleText = "Hello Mochi");
    void consoleView();
    
  private:
    const char* _consoleText;
    void commands_envio(const String& consoleText);
};
 
#endif

