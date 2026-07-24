//CLASS DEFINIDA POR LUCAS ROCHA
#include "Console.h"
Console console = Console("Mochi> ");


const char* hours_wakeon = "06:30";  // hours_down: Deve ser o fim do período noturno (06:00).
const char* hours_sleep = "22:03";   // hours_up: Deve ser o início do período noturno (22:00).
#include "Hours_Time.h"

#include "Animations.h"
Animations animations_exec;

// Passa a referência da animação para o relógio
Hours_Time hours_Time_exec = Hours_Time(hours_sleep, hours_wakeon, "", -3 * 3600, 0, "pool.ntp.org", &animations_exec);

#include "WirelessConnection.h"
#include "servidorweb.h"
#include "SDData.h"
SDData SDData_exec;

#include "MochiWebSocketClient.h"  // Inclui a classe cliente criada para o MochiDB
// Instancia o cliente WebSocket passando a referência da tela
MochiWebSocketClient wsClient(&animations_exec);

const char* SSID = "PERIGO";
const char* PASSWORD = "LIBER@RWIFI";

WirelessConnection wirelessConnection = WirelessConnection(SSID, PASSWORD, &animations_exec);

void startWifi() {
  wirelessConnection.backupRede();
  wirelessConnection.connections_Wifi();  // CONNECT WIFI
  wirelessConnection.searchRedes();       // SCAN WIFI REDE
  //wirelessConnection.connections_status();
  //wirelessConnection.btClassicScan();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  // Inicialização física do Cartão SD e listagem
  SDData_exec.sdbegin();
  SDData_exec.listDir("/", 1);
  animations_exec.helloWordMochi();                         // Inicializa a tela com o Hello Word do seu Mochi
  startWifi();                                              // Gerencia as conexões de rádio
  hours_Time_exec.time_server();                            // Configurações de hora baseadas no NTP Server
  wirelessConnection.Uptime();                              // ALARMES DE QUEDAS UP/DOWN
  startServer();                                            // Inicializa o servidor HTTP assíncrono (método do servidorweb.h)
  wsClient.begin("192.168.1.252", 8003, "/ws/animations");  // CONECTA AO SEU BACKEND DO MOCHIDB (Ajuste o IP e a porta se necessário)
  console.helloWord();                                      // CONSOLE
}

void loop() {
  // Se o Wi-Fi estiver conectado, escuta as strings do MochiDB e processa na tela
  if (WiFi.status() == WL_CONNECTED) {
    wsClient.loop();
    wsClient.verificarFluxoDados();
    animations_exec.processHexFrameLoop();  // Renderiza o frame Hex recebido do Mongo
  } else {
    // Se não houver internet/servidor, roda a animação padrão local em loop
    animations_exec.animationsLoop();
  }

  hours_Time_exec.weke_on();
  console.consoleView();

  delay(1);  // Respiro essencial para o núcleo do ESP32-C3
}
