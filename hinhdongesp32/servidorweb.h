#ifndef SERVIDORWEB_H
#define SERVIDORWEB_H

#include "SD.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h> // Instale a biblioteca ArduinoJson

#include "WirelessConnection.h"
extern WirelessConnection wirelessConnection;
#include "Hours_Time.h"
extern Hours_Time hours_Time_exec;

// Cria o objeto Servidor na porta 80 (porta HTTP padrão)
AsyncWebServer server(80);

bool wifiState = false;
bool bluetoothState = false;
bool webserverState = false;

// Protótipos de Funções
String getSwitchState(bool state);
String processor(const String& var);
void handleToggle(AsyncWebServerRequest *request);
void handleDateTime(AsyncWebServerRequest *request);

//Nova conexão
String pendinSsid = "";
String pendingPass = "";

String getSwitchState(bool state) {
    return state ? "checked" : ""; // Retorna "checked" se o estado for true (ligado), ou uma string vazia se for false (desligado)
}
// Função de Processamento para substituir o marcador %STATE% no HTML
String processor(const String& var){
    Serial.print("Placeholder requisitado: "); Serial.println(var);

    if(var == "WIFI_STATE") return getSwitchState(wifiState); // O HTML usa "checked" para ligar o switch
    
    if(var == "BLUETOOTH_STATE") return getSwitchState(bluetoothState);

    if(var == "WEBSERVER_STATE") return getSwitchState(webserverState);

    if(var == "SSID_VALUE") return (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : "Desconectado"; // Retorna o SSID atual ou uma mensagem se desconectado
    
    if(var == "IP_VALUE") return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "0.0.0.0"; // Retorna o IP local
    
    if(var == "MAC_VALUE") return WiFi.macAddress(); // Retorna o endereço MAC
    
    if(var == "WAKEON_DISPLAY") return String(hours_Time_exec.getHoursWakeon()); // Retorna o valor da função getHoursWakeon()
    
    if(var == "SLEEP_DISPLAY") return String(hours_Time_exec.getHoursSleep()); // Retorna o valor da função getHoursSleep()

    // Para qualquer outro placeholder não mapeado
    return String();
}

// Função que lida com a alternância de estado de qualquer switch
void handleToggle(AsyncWebServerRequest *request) {
    String param = "state";
    String value = "toggle";

    if (request->hasParam(param)) {
        value = request->getParam(param)->value();
    }

    Serial.print("Comando de alternância recebido: ");
    Serial.println(value);

    // Verifica qual switch deve ser alternado
    if (value == "wifi") {
        wifiState = !wifiState;
        Serial.printf("WiFi alternado para: %s\n", wifiState ? "LIGADO" : "DESLIGADO");
        // Coloque aqui a lógica para iniciar/parar o WiFi se necessário
        WiFi.disconnect(true); // Desconecta da rede
        Serial.println("Desligando WiFi...");
        WiFi.mode(WIFI_OFF); // Desliga a interface Wi-Fi
        Serial.println("WiFi desligado.");
    } 
    else if (value == "bluetooth") {
        bluetoothState = !bluetoothState;
        Serial.printf("Bluetooth alternado para: %s\n", bluetoothState ? "LIGADO" : "DESLIGADO");
        // Coloque aqui a lógica para iniciar/parar o Bluetooth se necessário
    }
    else if (value == "webserver") {
        webserverState = !webserverState;
        Serial.printf("Servidor Web alternado para: %s\n", webserverState ? "LIGADO" : "DESLIGADO");
        // Lembre-se: Desligar o servidor web exige reinício ou parada forçada, cuidado!
    }

    // Após alternar o estado, redireciona o usuário de volta para a página inicial
    request->redirect("/");
}

void handleDateTime(AsyncWebServerRequest *request) {
    // Obter o tempo atual do sistema (assumindo que o NTP foi configurado)
    time_t now = time(nullptr); 
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    // Alocação de memória para strings formatadas
    char dateString[11]; // DD/MM/YYYY + '\0'
    char hourString[9]; // HH:MM:SS + '\0'

    // Formata a data e hora
    strftime(dateString, sizeof(dateString), "%d/%m/%Y", &timeinfo);
    strftime(hourString, sizeof(hourString), "%H:%M:%S", &timeinfo);
    
    // Constrói a resposta JSON para o JavaScript
    String responseJson = "{\"date\":\"";
    responseJson += dateString;
    responseJson += "\",\"time\":\"";
    responseJson += hourString;
    responseJson += "\"}";

    // Envia a resposta JSON. É ESSENCIAL usar send() aqui.
    request->send(200, "application/json", responseJson);
}

void startServer() {
    // Configuração de Rotas (Endpoints)
    server.serveStatic("/www/", SD, "/www/");
    // Rota raiz (/) - Lendo do SD
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        // Verifica se o arquivo existe no SD
        if (SD.exists("/www/index.html")) {
            // Enviamos o arquivo do SD passando o processor para templates
            request->send(SD, "/www/index.html", "text/html", false, processor);
        } else {
            request->send(404, "text/plain", "Arquivo nao encontrado no SD");
        }
    });
    // Rota de alternância (Toggle) para todos os switches
    server.on("/toggle", HTTP_GET, handleToggle);
    
    // NOVO: Rota para Data e Hora Dinâmicas (JSON)
    server.on("/datetime", HTTP_GET, handleDateTime); // <--- Adicione esta linha

    // Rota que o JavaScript vai chamar para obter a lista de redes
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
        int n = WiFi.scanComplete(); // Verifica se um scan já foi feito
        // Se o scan não foi iniciado (-2) ou está rodando (-1)
        if(n < 0) {
            if(n == -2) {
                // Limpa scans anteriores e inicia um novo em background (true)
                WiFi.scanNetworks(true); 
            }
            // Retorna 202 (Accepted) para dizer ao JS: "Recebi, mas volte mais tarde"
            request->send(202, "application/json", "{\"status\":\"scanning\"}");
            return;
        }
        JsonDocument doc; 
        JsonArray root = doc.to<JsonArray>();

        for (int i = 0; i < n; ++i) {
            JsonObject item = root.add<JsonObject>();
            item["ssid"] = WiFi.SSID(i);
            item["rssi"] = WiFi.RSSI(i);

            // Identificação detalhada da segurança
            uint8_t type = WiFi.encryptionType(i);
            String encDesc = wirelessConnection.getEncryptionName(type);
            
            item["enc"] = encDesc;
            // Adiciona um booleano para facilitar o filtro no Frontend
            item["isWPA3"] = (type == WIFI_AUTH_WPA3_PSK || type == WIFI_AUTH_WPA2_WPA3_PSK);
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
        // Deleta os resultados da memória para o próximo scan ser limpo
        WiFi.scanDelete();
    });

    server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid") && request->hasParam("pass")) {
            String pendingSsid = request->getParam("ssid")->value();
            String pendingPass = request->getParam("pass")->value();
            
            WiFi.disconnect();
            WiFi.begin(pendingSsid.c_str(), pendingPass.c_str());

            request->send(200, "application/json", "{\"status\":\"attempting\", \"msg\":\"Tentando conectar...\"}");
        } else {
            request->send(400, "text/plain", "Faltam parametros");
        }

    });
    // Inicia o Servidor 
    server.begin();
    Serial.println("Servidor HTTP Async Iniciado!");
}

void run(){
  // Com o ESPAsyncWebServer, o loop() fica livre para outras tarefas
  // Nenhuma chamada de 'server.handleClient()' é necessária!
  // Podemos, por exemplo, ler um sensor aqui a cada 1 segundo:
  // delay(1000);
}

#endif