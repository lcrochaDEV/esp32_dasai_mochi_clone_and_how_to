//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console;


const char* hours_wakeon = "06:30"; // hours_down: Deve ser o fim do período noturno (06:00).
const char* hours_sleep = "22:03";   // hours_up: Deve ser o início do período noturno (22:00).
#include "Hours_Time.h"
Hours_Time hours_Time_exec = Hours_Time(hours_sleep, hours_wakeon);

#include "Animations.h"
Animations animations_exec;

#include "WirelessConnection.h"
#include "servidorweb.h"
#include "SDData.h"
SDData SDData_exec;

const char* SSID = "PERIGO";
const char* PASSWORD = "LIBER@RWIFI";

WirelessConnection wirelessConnection = WirelessConnection(SSID, PASSWORD);

void startWifi() {
  wirelessConnection.backupRede();
  wirelessConnection.connections_Wifi(); // CONNECT WIFI
  wirelessConnection.searchRedes(); //SCAN WIFI REDE
  wirelessConnection.connections_status();
}

void setup() {
  Serial.begin(115200);
  SDData_exec.sdbegin();
  SDData_exec.listDir("/", 1);  
  animations_exec.helloWordMochi(); //HELLO WORD
  startWifi();
  hours_Time_exec.time_server(); // TIME
  wirelessConnection.Uptime();
  startServer(); // START SERVIDOR WEB
}

void loop() {
  animations_exec.animationsLoop();
  hours_Time_exec.weke_on();
  //CONSOLE
  console.consoleView();
} 