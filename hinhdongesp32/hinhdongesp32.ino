//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console;

#include "Hours_Time.h"
Hours_Time hours_Time;

#include "Animations.h"
Animations animations;

#include "ConnectWifi.h"

const char* SSID = "PERIGO";
const char* PASSWORD = "LIBER@RWIFI3";

ConnectWifi connectWifi = ConnectWifi(SSID, PASSWORD);

void setup() {
  Serial.begin(115200);
  connectWifi.connectionsMethod();

  animations.helloWordMochi();
  hours_Time.time_server();
}

void loop() {
  //animations.animationsLoop();

  //animations.animeScreen("Mochi");

  bool nonect_wifi = connectWifi.connect_status();
  if(nonect_wifi == true){
    Serial.println("Conectado com sucesso!");
  }else{
    animations.not_wifi();
  }

  //CONSOLE
  console.consoleView();
}

