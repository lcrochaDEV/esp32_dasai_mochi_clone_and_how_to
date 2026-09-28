#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>
#include <stdarg.h>

#include "WirelessConnection.h"
#include "SDData.h"
#include "Animations.h"
#include "MochiWebSocketClient.h"

class Console: public WirelessConnection, public SDData, public Animations, public MochiWebSocketClient {
  public:
    Console(const char* consoleText = "Mochi> ");
    
    // Métodos legados/compatibilidade
    void helloWord(const char* consoleText = nullptr);
    void menssageViewMsg(const char* consoleText = nullptr);
    void consoleView();

    // Sistema de Log Unificado
    void log(const char* message);
    void log(const String& message);
    void logf(const char* format, ...);
    void setLogState(bool enable);
    bool isLogEnabled() const;

  private:
    const char* _consoleText;
    bool _logsEnabled;
    
    void commands_envio(const String& command);
    void printPrompt();
};


extern Console console;
#endif