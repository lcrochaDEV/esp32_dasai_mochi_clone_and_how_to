#include "Console.h"

Console::Console(const char* consoleText) 
  : WirelessConnection(), 
    SDData(), 
    Animations(), 
    MochiWebSocketClient(this), 
    _consoleText(consoleText != nullptr ? consoleText : "Mochi> "),
    _logsEnabled(false)
{}

void Console::helloWord(const char* consoleText) {
  const char* msg = (consoleText != nullptr) ? consoleText : _consoleText;
  if (msg != nullptr) menssageViewMsg(msg);
}

void Console::menssageViewMsg(const char* consoleText) {
  if (consoleText != nullptr) Serial.println(consoleText);
}

// Imprime o prompt sem criar novas linhas
void Console::printPrompt() {
  if (_consoleText != nullptr) Serial.print(_consoleText);
}

// Habilita ou desabilita os logs sem disparar printPrompt extra
void Console::setLogState(bool enable) {
  _logsEnabled = enable;
  String status = _logsEnabled ? "[SYSTEM] Logs Habilitados." : "[SYSTEM] Logs Deshabilitados.";
  menssageViewMsg(status.c_str());
  // Removido o printPrompt() daqui para evitar duplicar Mochi>
}

bool Console::isLogEnabled() const {
  return _logsEnabled;
}

// Método central para canalizar logs de todas as classes
void Console::log(const char* message) {
  if (!_logsEnabled || message == nullptr) return;

  // Utiliza variável estática para rastrear se é o primeiro log após o prompt
  static bool firstLogAfterPrompt = true;

  if (firstLogAfterPrompt) {
    // Na primeira execução após habilitar o log/prompt, pula a linha para sair de baixo do "Mochi> "
    Serial.print("\n[LOG] ");
    Serial.print(message);
    firstLogAfterPrompt = false; // Desativa a quebra extra para os logs subsequentes
  } else {
    // Logs normais em sequência: colados linha a linha sem quebras extras
    Serial.print("[LOG] ");
    Serial.print(message);
  }
}

void Console::logf(const char* format, ...) {
  if (!_logsEnabled || format == nullptr) return;

  static char buffer[128]; 
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  this->log((const char*)buffer);
}

void Console::log(const String& message) {
  this->log(message.c_str());
}

void Console::consoleView() {
  static String inputBuffer = "";

  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      inputBuffer.trim();
      
      if (inputBuffer.length() > 0) {
        inputBuffer.toUpperCase();
        commands_envio(inputBuffer);
      } else {
        printPrompt();
      }
      
      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }
}




void Console::commands_envio(const String& command) {
  // 1. Extrai o comando base e o parâmetro opcional caso tenha espaço (ex: "SETDELAY 0.09")
  String cmd = command;
  String paramStr = "";
  int spaceIdx = command.indexOf(' ');

  if (spaceIdx != -1) {
    cmd = command.substring(0, spaceIdx);
    paramStr = command.substring(spaceIdx + 1);
    paramStr.trim();
  }

  // Mostra o comando digitado pelo usuário
  Serial.print(_consoleText);
  Serial.println(command);

  if (cmd == "HELP") {
      menssageViewMsg(
        "\n--- LISTA DE COMANDOS ---\n"
        "HELP       - Exibe a lista de comandos e funcionalidades\n"
        "LOGON      - Habilita a exibição dos logs do sistema\n"
        "LOGOFF     - Desabilita a exibição dos logs do sistema\n"
        "SCANWF     - Realiza a varredura de redes Wi-Fi disponíveis\n"
        "SHOWDATA   - Exibe os dados do cartão SD em formato JSON\n"
        "DELETEDATA - Apaga os arquivos de dados salvos no cartão SD\n"
        "DISPLAYON  - Liga a tela do display OLED\n"
        "DISPLAYOFF - Desliga a tela do display OLED\n"
        "ANIMACAO   - Executa a animação padrão no display OLED\n"
        "SETDELAY   - Ajusta a velocidade da animação (Ex: SETDELAY 0.09)"
      );
  }
  // Tratamento do ajuste de delay / velocidade da animação
  else if (cmd == "SETDELAY" || cmd == "DELAY") {
    float delayVal = 0.09f; // Valor padrão inicial de 0.09s se nada for passado
    
    if (paramStr.length() > 0) {
      delayVal = paramStr.toFloat();
    }
    
    menssageViewMsg(("Ajustando velocidade para " + String(delayVal, 4) + "s...").c_str());
    this->setAnimationDelay(delayVal);
  }
  // Controle de Logs
  else if (cmd == "LOGON") setLogState(true);
  else if (cmd == "LOGOFF") setLogState(false);
  
  // Wi-Fi & Redes
  else if (cmd == "SCANWF") this->searchRedes();
  
  // SD & Data
  else if (cmd == "SHOWDATA") printJSON();
  else if (cmd == "DELETEDATA") deleteArquivo();
  
  // Display
  else if (cmd == "DISPLAYON") control_oled_power(true);
  else if (cmd == "DISPLAYOFF") control_oled_power(false);
  
  // Animação
  else if (cmd == "ANIMACAO") defaultlocal();
  
  else {
    menssageViewMsg("Comando inexistente. Digite HELP.");
  }
  
  // Desenha o prompt uma única vez ao final de cada comando processado
  printPrompt();
}