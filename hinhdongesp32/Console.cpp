#include <algorithm>
#include "Console.h"
#include "Animations.h"


//MENSAGEM DE TODO O PROGRAMA
Console::Console(const char* consoleText)
  : consoleText(consoleText)
{}
void Console::helloWord(String consoleText) {
  menssageViewMsg(consoleText);
}
void Console::menssageViewMsg(String consoleText) {
  Serial.println(consoleText);
}
void Console::consoleView() {
  if (Serial.available() > 0) {
    String consoleText = Serial.readString();
    consoleText.trim();
    if (consoleText.length() > 0) {
      consoleText.toUpperCase();
      commands_envio(consoleText);
    }
  }
}

void Console::commands_envio(String consoleText) {
  menssageViewMsg("Mochi> " + consoleText);
  if (consoleText == "SCAN"){
    this->searchRedes();
  }
}
