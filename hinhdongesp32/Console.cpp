#include "Console.h"

Console::Console(const char* consoleText) 
  : WifiConnect(), consoleText(consoleText) 
{}

void Console::helloWord(const char* consoleText) {
  // Se o argumento 'text' for nulo, usamos o 'consoleText' da classe
  const char* msg = (consoleText != nullptr) ? consoleText : this->consoleText;
  
  if (msg != nullptr) {
    menssageViewMsg(msg);
  }
}

void Console::menssageViewMsg(const char* consoleText) {
  Serial.println(consoleText);
}

void Console::consoleView() {
  if (Serial.available() > 0) {
    // readStringUntil evita atrasos de timeout do readString comum
    String consoleText = Serial.readStringUntil('\n');
    consoleText.trim();
    
    if (consoleText.length() > 0) {
      consoleText.toUpperCase();
      commands_envio(consoleText.c_str());
    }
  }
}

void Console::commands_envio(const char* comando) {
  String prefixo = "Mochi> ";
  menssageViewMsg((prefixo + comando).c_str());

  if (comando == "SCAN") {
    // Chama o método herdado de WifiConnect
    this->searchRedes(); 
  } 
  else if (comando == "HELP") {
    menssageViewMsg("Comandos: SCAN, HELP");
  } 
  else {
    menssageViewMsg("Comando inexistente. Digite HELP.");
  }
}