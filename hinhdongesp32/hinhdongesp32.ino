//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console;


const char* hours_down = "22:00";
const char* hours_up = "06:30";
#include "Hours_Time.h"
Hours_Time hours_Time_exec = Hours_Time(hours_up, hours_down);

#include "Animations.h"
Animations animations_exec;

#include "WifiConnect.h"

const char* SSID = "PERIGO";
const char* PASSWORD = "LIBER@RWIFI";

WifiConnect wifiConnect = WifiConnect(SSID, PASSWORD);


void setup() {
  Serial.begin(115200);
  animations_exec.helloWordMochi();
  wifiConnect.connectionsMethod();
  hours_Time_exec.time_server();
}

void loop() {
  animations_exec.animationsLoop();
  hours_Time_exec.weke_on();
  //CONSOLE
  console.consoleView();
}