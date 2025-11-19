#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>

class Console {
  public:
    Console(String consoleText = "");
    //MENSAGEM DE TODO O PROGRAMA
    void helloWord(String consoleText = "Hello Mochi");
    void menssageViewMsg(String consoleText);
    void consoleView();
  private:
    String consoleTextView;
    void commands_envio(String consoleText);
};
 
#endif