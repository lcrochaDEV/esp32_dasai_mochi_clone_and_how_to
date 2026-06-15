//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console = Console("Mochi> ");


const char* hours_wakeon = "06:30";  // hours_down: Deve ser o fim do período noturno (06:00).
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

WirelessConnection wirelessConnection = WirelessConnection(SSID, PASSWORD, &animations_exec);

void startWifi() {
  wirelessConnection.backupRede();
  wirelessConnection.connections_Wifi();  // CONNECT WIFI
  wirelessConnection.searchRedes();       //SCAN WIFI REDE
  //wirelessConnection.connections_status();
  //wirelessConnection.btClassicScan();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  // Inicialização física do Cartão SD e listagem
  SDData_exec.sdbegin();
  SDData_exec.listDir("/", 1);
  animations_exec.helloWordMochi();  // Inicializa a tela com o Hello Word do seu Mochi
  startWifi(); // Gerencia as conexões de rádio
  hours_Time_exec.time_server();  // Configurações de hora baseadas no NTP Server
  wirelessConnection.Uptime();    // ALARMES DE QUEDAS UP/DOWN
  startServer();                 // Inicializa o servidor HTTP assíncrono (método do servidorweb.h)
  console.helloWord(); //CONSOLE
}

void loop() {
  animations_exec.animationsLoop();
  hours_Time_exec.weke_on();
  //CONSOLE
  console.consoleView();
  delay(1);
}

