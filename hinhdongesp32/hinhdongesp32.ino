//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console;

#include "Animations.h"
Animations animations;

void setup() {
  animations.helloWordMochi();
}

void loop() {
  animations.animationsLoop();

  //animations.animeScreen("Mochi");
  //CONSOLE
  console.consoleView();
}

