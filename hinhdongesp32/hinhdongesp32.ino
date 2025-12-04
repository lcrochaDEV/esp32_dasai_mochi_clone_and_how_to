//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console;

#include "Hours_Time.h"
Hours_Time hours_Time;

#include "Animations.h"
Animations animations;

#include "WifiConnect.h"

const char* SSID = "PERIGO";
const char* PASSWORD = "LIBER@RWIFI";

WifiConnect wifiConnect = WifiConnect(SSID, PASSWORD);


void setup() {
  Serial.begin(115200);
  wifiConnect.connectionsMethod();
  animations.helloWordMochi();
  hours_Time.time_server();
}

void loop() {
  bool nonect_wifi = wifiConnect.connect_status();
  if(nonect_wifi == true){
    Serial.println("Conectado com sucesso!");
    animations.animationsLoop();
  }else{
    animations.not_wifi();
  }
  //delay(5000);
  //CONSOLE
  console.consoleView();
}

