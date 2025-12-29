#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>

#include "WifiConnect.h"

class Console: public WifiConnect{
  public:
    Console(const char* consoleText = nullptr);
    //MENSAGEM DE TODO O PROGRAMA
    void helloWord(const char* consoleText = "Hello Mochi");
    void menssageViewMsg(const char* consoleText = "Hello Mochi");
    void consoleView();

  private:
    const char* consoleText;
    void commands_envio(const char* consoleText);
};
 
#endif
