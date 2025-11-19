#include "Console.h"
#include "Animations.h"

//extern Animations animations;

//MENSAGEM DE TODO O PROGRAMA
Console::Console(String consoleText){
    this->consoleTextView = consoleText;
}
void Console::helloWord(String consoleText){
    menssageViewMsg(consoleText);
}
void Console::menssageViewMsg(String consoleText = "Hello Mochi"){
    Serial.println(consoleText);
}
void Console::consoleView(){
  if (Serial.available() > 0) {
    String consoleText = Serial.readString();
    consoleText.trim();
    if (consoleText.length() > 0) {
      consoleText.toUpperCase();
      commands_envio(consoleText);
    }
  }
}

void Console::commands_envio(String consoleText){
    menssageViewMsg(consoleText);
   // animations.prompt_command("helllo");
}