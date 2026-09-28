#include "Console.h"

Console::Console(const char* consoleText) 
  : WirelessConnection(), 
    SDData(), 
    Animations(), 
    MochiWebSocketClient(this), 
    _consoleText(consoleText != nullptr ? consoleText : "Mochi> "),
    _logsEnabled(true) // Logs ativados por padrão na inicialização
{
}

void Console::helloWord(const char* consoleText) {
  const char* msg = (consoleText != nullptr) ? consoleText : _consoleText;
  if (msg != nullptr) menssageViewMsg(msg);
}

void Console::menssageViewMsg(const char* consoleText) {
  if (consoleText != nullptr) {
    Serial.println(consoleText);
  }
}

// Imprime o prompt definido na inicialização pelo usuário
void Console::printPrompt() {
  if (_consoleText != nullptr) {
    Serial.print(_consoleText);
  }
}

// Habilita ou desabilita os logs de todas as classes
void Console::setLogState(bool enable) {
  _logsEnabled = enable;
  String status = _logsEnabled ? "[SYSTEM] Logs Habilitados." : "[SYSTEM] Logs Deshabilitados.";
  menssageViewMsg(status.c_str());
  printPrompt();
}

bool Console::isLogEnabled() const {
  return _logsEnabled;
}

// Método central para canalizar logs de todas as classes
void Console::log(const char* message) {
  if (!_logsEnabled || message == nullptr) return;
  
  Serial.println(); // Garante que quebra a linha do prompt se necessário
  Serial.print("[LOG] ");
  Serial.println(message);
  printPrompt(); // Exibe o prompt novamente após a mensagem de log
}

void Console::logf(const char* format, ...) {
  if (!_logsEnabled || format == nullptr) return;

  // Buffer estático/reduzido para não estourar a pilha da Task/FreeRTOS
  static char buffer[128]; 
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  this->log((const char*)buffer);
}

void Console::log(const String& message) {
  log(message.c_str());
}

void Console::consoleView() {
  static String inputBuffer = ""; // Guarda os caracteres à medida que chegam

  // Lê todos os caracteres disponíveis no buffer Serial sem bloqueio
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') { // Se pressionou ENTER (fim do comando)
      inputBuffer.trim();
      
      if (inputBuffer.length() > 0) {
        inputBuffer.toUpperCase();
        commands_envio(inputBuffer);
      } else {
        printPrompt();
      }
      
      inputBuffer = ""; // Limpa o buffer para o próximo comando
    } else {
      inputBuffer += c; // Concatena os caracteres digitados
    }
  }
}

void Console::commands_envio(const String& command) {
  // Echo do comando do usuário acompanhado do prompt
  menssageViewMsg((String(_consoleText) + command).c_str());

  if (command == "HELP") {
    menssageViewMsg("Comandos: SHOWDATA, HELP, SCANWF, DELETEDATA, DISPLAYON, DISPLAYOFF, ANIMACAO, LOGON, LOGOFF");
  }
  // Controle de Logs
  else if (command == "LOGON") setLogState(true);
  
  else if (command == "LOGOFF") setLogState(false);
  
  // Wi-Fi & Redes
  else if (command == "SCANWF") this->searchRedes();
  
  // SD & Data
  else if (command == "SHOWDATA") printJSON();
  
  else if (command == "DELETEDATA") deleteArquivo();
  
  // Display
  else if (command == "DISPLAYON") control_oled_power(true);
  
  else if (command == "DISPLAYOFF") control_oled_power(false);
  
  // Animação
  else if (command == "ANIMACAO") defaultlocal();
  
  else menssageViewMsg("Comando inexistente. Digite HELP.");
  
  // Imprime o prompt do usuário pronto para o próximo comando
  printPrompt();
}