#ifndef SERVIDORWEB_H
#define SERVIDORWEB_H

#include <WebServer.h>
#include <uri/UriBraces.h>
#include <WiFi.h>

#include "Hours_Time.h"
Hours_Time timeManager;

WebServer server(80);
bool wifiState = false;
bool bluetoothState = false;
bool webserverState = false;

String getSwitchState(bool state) {
    // Retorna "checked" se o estado for true (ligado), ou uma string vazia se for false (desligado)
    return state ? "checked" : ""; 
}

void sendHtml() {
    const char* sleepTime = timeManager.getHoursSleep();
    const char* wakeTime = timeManager.getHoursWakeon();

    String response = R"(
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta http-equiv="X-UA-Compatible" content="IE=edge">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined:opsz,wght,FILL,GRAD@20..48,100..700,0..1,-50..200" />
            <title>Mochi D.I.Y</title>
            <style>
                :root {
                    --color-font: #ffffff;
                    --color-boder: rgba(148, 141, 135, 0.167);
                    --bg-header: rgba(148, 141, 135, 0.167);
                    --accent-light: #a78bfa;
                }
                * {
                    margin: 0;
                    padding: 0;
                    box-sizing: content-box;
                    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                }
                html {
                    background: rgba(26, 13, 46, 0.95);
                }
                header {
                    width: 100%;
                    background-color: var(--bg-header);
                    backdrop-filter: blur(10px);
                    opacity: .7;
                    color: var(--color-font);
                }
                .img {
                    display: flex;
                    justify-content: space-between;
                }
                .mochi, .dasai {
                    width: 100px;
                    margin: 5px;
                }
                section {
                    display: flex;
                    justify-content: start;
                    color: var(--color-font);
                    margin-top: 10px;
                    margin-left: 5px;
                }
                .conteiner{
                    background-color: var(--bg-header);
                    border-radius: 5px;
                    margin-right: 12px;
                    border: 1px solid;
                    border-color: var(--accent-light);
                    box-shadow: 0 0 20px rgba(139, 92, 246, 0.4);
                }
                section h3, p {
                    margin: 10px 15px;
                }
                .conteiner span {
                    font-weight: bold;
                }
                .conteiner_sw {
                    display: flex;
                    justify-content: space-between;
                    margin: 0 10px 10px 0;
                }
                .switch {
                    margin-left: -9999px;
                    visibility: hidden;
                    position: relative;
                }

                .switch + label {
                    display: block;
                    position: relative;
                    cursor: pointer;
                    outline: none;
                    user-select: none;
                }
                .switch--shadow + label {
                    padding: 2px;
                    width: 35px;
                    height: 20px;
                    background-color: #dddddd;
                    border-radius: 30px;
                }
                .switch--shadow + label:before,
                .switch--shadow + label:after {
                    display: block;
                    position: absolute;
                    top: 1px;
                    left: 1px;
                    bottom: 1px;
                    content: '';
                }
                .switch--shadow + label:before {
                    right: 1px;
                    background-color: #f1f1f1;
                    border-radius: 30px;
                    transition: all 0.4s;
                }
                .switch--shadow + label:after {
                    width: 30px;
                    background-color: #fff;
                    border-radius: 100%;
                    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.3);
                    transition: all 0.4s;
                }
                .switch--shadow:checked + label:before {
                    background-color: #8ce196;
                }
                .switch--shadow:checked + label:after {
                    transform: translateX(8px);
                }
            </style>
        </head>
        <body>
            <header>
                <div class="img">
                    <img class="mochi" src="https://dasai.com.au/cdn/shop/files/logo_2.png?v=1746165958&width=4370" alt="mochi">
                    <img class="dasai" src="https://tenereteam.s3.us-west-1.amazonaws.com/dasai-logo-updated.png?v=1762691630" alt="dasai">
                </div>
            </header>
            <section>
                <div class="conteiner">
                    <h3>Redes</h3>
                    <div class="conteiner_sw">
                        <p><span>WIFI:</span></p>
                        <div class="switch__container">
                            <input id="switch-wifi" class="switch switch--shadow" type="checkbox" WIFI_STATE/>
                            <label for="switch-wifi"></label>
                        </div>
                    </div>
                    <p><span>SSID: </span>SSID_VALUE</p>
                    <p><span>Password:</span> *******</p>
                    <p><span>IP Address: </span>IP_VALUE</p>
                    <p><span>MAC Address: </span>MAC_VALUE</p>
                    <div class="conteiner_sw">
                        <p><span>Bluetooth:</span></p>
                        <div class="switch__container">
                            <!-- Este agora é exclusivo para Bluetooth -->
                            <input id="switch-bluetooth" class="switch switch--shadow" type="checkbox" BLUETOOTH_STATE/>
                            <label for="switch-bluetooth"></label>
                        </div>
                    </div>
                </div>
                <div class="conteiner">
                    <h3>Configurações</h3>
                    <div class="conteiner_sw">
                        <p><span>Servidor Web:</span></p>
                        <div class="switch__container">
                            <!-- Este agora é exclusivo para Servidor Web -->
                            <input id="switch-webserver" class="switch switch--shadow" type="checkbox" WEBSERVER_STATE/>
                            <label for="switch-webserver"></label>
                        </div>
                    </div>
                    <h3>Wakeon Mode</h3>
                    <p><span>Wakon: </span>WAKEON_DISPLAY</p>
                    <p><span>Sleep: </span>SLEEP_DISPLAY</p>
                </div>
                <div class="conteiner">
                </div>
            </section>
        </body>
        </html>
    )";
// --- INJEÇÃO DE VALORES DINÂMICOS ---
    response.replace("WIFI_STATE", getSwitchState(wifiState));
    response.replace("BLUETOOTH_STATE", getSwitchState(bluetoothState));
    response.replace("WEBSERVER_STATE", getSwitchState(webserverState));
    
    response.replace("SSID_VALUE", WiFi.SSID());
    response.replace("IP_VALUE", WiFi.localIP().toString());
    response.replace("MAC_VALUE", WiFi.macAddress());

    response.replace("WAKEON_DISPLAY", wakeTime);
    response.replace("SLEEP_DISPLAY", sleepTime);

    server.send(200, "text/html", response);
}

void handleToggle() {
    if (server.hasArg("device") && server.hasArg("state")) {
        String device = server.arg("device");
        String stateStr = server.arg("state");
        bool newState = (stateStr == "on");

        if (device == "wifi") {
            wifiState = newState;
            // Aqui você deve adicionar a lógica de conexão/desconexão real do WiFi (WiFi.begin(), WiFi.disconnect(), etc.)
            Serial.print("WiFi Toggled: "); Serial.println(wifiState);
        } else if (device == "bluetooth") {
            bluetoothState = newState;
            // Aqui você deve adicionar a lógica de ativação/desativação do Bluetooth (btStart(), btStop(), etc.)
            Serial.print("Bluetooth Toggled: "); Serial.println(bluetoothState);
        } else if (newState == false) {
                // Adicional: Desliga o servidor se o switch for desativado
                // Note: Você precisará de uma forma de religá-lo ou reiniciar o ESP32
                server.stop(); 
                Serial.println("Webserver stopped manually.");
            } else {
                 // Nota: Tentar iniciar server.begin() novamente pode não funcionar bem.
                 // Para ligar novamente, você pode precisar de uma função de reset ou uma verificação
            }
    }
    // Redireciona o cliente para a página inicial para atualizar a UI
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

// Função de Configuração (Implementação com argumentos)
void startServer(bool initialWifiState) {
  // Inicializa as variáveis de estado com os valores passados
  wifiState = initialWifiState;
  
  // 2. Configuração de Rotas do Servidor Web
  server.on("/", sendHtml);
  server.on("/toggle", handleToggle);
  // 3. Inicia o Servidor
  server.begin();
  webserverState = true;
  Serial.println("HTTP server started");
}

void run(void) {
  server.handleClient();
  delay(2);
}

#endif