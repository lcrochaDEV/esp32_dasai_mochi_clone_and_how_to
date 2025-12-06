//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console;


const char* hours_down = "19:18";
const char* hours_up = "06:30";
#include "Hours_Time.h"
Hours_Time hours_Time = Hours_Time(hours_up, hours_down);

#include "Animations.h"
Animations animations;

#include "WifiConnect.h"

const char* SSID = "PERIGO";
const char* PASSWORD = "LIBER@RWIFI2";

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
    animations.animationsLoop();
  }else{
    animations.not_wifi();
  }
  hours_Time.weke_on();
  //CONSOLE
  console.consoleView();
}
