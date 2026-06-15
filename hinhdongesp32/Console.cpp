#include "Console.h"

Console::Console(const char* consoleText) 
  : WirelessConnection(), SDData(), _consoleText(consoleText) 
{}

void Console::helloWord(const char* consoleText) {
  // Se o argumento 'text' for nulo, usamos o 'consoleText' da classe
  const char* msg = (consoleText != nullptr) ? consoleText : _consoleText;
  
  if (_consoleText) return menssageViewMsg(_consoleText);
  if (msg != nullptr) menssageViewMsg(msg);
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

void Console::commands_envio(const String& command) {
  String prefixo = "Mochi> ";
  menssageViewMsg((prefixo + command).c_str());

  if (command == "HELP") menssageViewMsg("Comandos: SHOWDATA, HELP");

  else if(command == "SCANWF") this->searchRedes(); // Chama o método herdado de WifiConnect
  
  else if(command == "ERASEDATA");
  
  //SSD COMMANDS
  else if(command == "SHOWDATA") printJSON();
  else if(command == "DELETEDATA") deleteArquivo();

  else menssageViewMsg("Comando inexistente. Digite HELP.");
  menssageViewMsg(_consoleText);
}
/*
1. Como fizemos em um outro projeto, e tendo um cartão sd para armazenamento de dados, 
vamos criar um estruturar o projeto para escrita, leitura, edição e deleção de dados, em um arquivo config.json que vai ser salvo dentro da pasta data.

Assim que algum dados forem salvos:
  1. Se o arquivo não exister vai ser criado
  2. Se existir so vamos manipular os dados.

2. Tenho o html na pasta /www, nela existe configurações onde temos Wakeon Mode, onde vai Wakon, Sleep, vão se tornar um imput para configurações de horar, e também uma tag de informações,
onde em cada campo vai ter a descrição de sua ultilidade, hora de acordar e dormir.

Após o cadastro das horas:

  1. Vamos salvar esses dados no arquivo config.json para analise.
  2. Se o modulo for desligado ele vai se lembrar dos dados assim que reiniciar.
  3. Se as horas forem editadas, esses dados vão ser modificados.

todas essas informações vão usar um codigo proficional e menos verboso usando a versão atual da bibliotéca json pra formatar os dado e recuperalos do arquivos json.
























*/