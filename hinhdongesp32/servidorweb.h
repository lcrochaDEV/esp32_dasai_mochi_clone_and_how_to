#ifndef SERVIDORWEB_H
#define SERVIDORWEB_H

#include <Arduino.h>
#include <ArduinoJson.h> // Instale a biblioteca ArduinoJson
#if defined(ESP8266)
  #include <ESPAsyncTCP.h>
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <AsyncTCP.h>
  #include <WiFi.h>
#endif

#include "SD.h"

#include <ESPAsyncWebServer.h>

#include "WirelessConnection.h"
extern WirelessConnection wirelessConnection;
#include "Hours_Time.h"
extern Hours_Time hours_Time_exec;
#include "AccessControl.h"
AccessControl accessSys; 
#include "EspMqtt.h"
EspMqtt mqttService;
#include "UUID.h"
UUID uuid;

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
String pendingSsid = "";
String pendingPass = "";

String getSwitchState(bool state) {
    return state ? "checked" : ""; // Retorna "checked" se o estado for true (ligado), ou uma string vazia se for false (desligado)
}
// Função de Processamento para substituir o marcador %STATE% no HTML
String processor(const String& var){
    Serial.print("Placeholder requisitado: "); 
    Serial.println(var);

    if(var == "WIFI_STATE") return getSwitchState(wifiState); // O HTML usa "checked" para ligar o switch
    if(var == "BLUETOOTH_STATE") return getSwitchState(bluetoothState);
    if(var == "WEBSERVER_STATE") return getSwitchState(webserverState);
    if(var == "SSID_VALUE") return (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : "Desconectado"; // Retorna o SSID atual ou uma mensagem se desconectado
    if(var == "IP_VALUE") return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "0.0.0.0"; // Retorna o IP local   
    if(var == "MAC_VALUE") return WiFi.macAddress(); // Retorna o endereço MAC    
    if(var == "WAKEON_DISPLAY") return String(hours_Time_exec.getHoursWakeon()); // Retorna o valor da função getHoursWakeon()
    if(var == "SLEEP_DISPLAY") return String(hours_Time_exec.getHoursSleep()); // Retorna o valor da função getHoursSleep()

    if(var == "MODULE_VALUE")         return accessSys.modelBoardESP();
    if(var == "TOTAL_RAN_VALUE")      return accessSys.total_ran();
    if(var == "FLASH_SIZE_VALUE")     return accessSys.flash_size();
    if(var == "MENOR_RAN_SIZE_VALUE") return accessSys.menor_ran_size();
    if(var == "SKETCH_SIZE_VALUE")    return accessSys.sketch_Size();
    // Para qualquer outro placeholder não mapeado
    return String("");
}

// Função que lida com a alternância de estado de qualquer switch
void handleToggle(AsyncWebServerRequest *request) {
    String param = "state";
    String value = "toggle";

    if (request->hasParam(param)) value = request->getParam(param)->value();

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
    // 1. CHECAGEM DE SEGURANÇA: O SD está montado?
    // Se o SD não puder ler a raiz, não deixamos o código prosseguir para evitar o crash.
    if (!SD.exists("/")) {
        Serial.println("⚠️ [ERRO CRÍTICO] Servidor não iniciado: Cartão SD inacessível ou não formatado!");
        return; 
    }
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
    server.on("/mqtt", HTTP_GET, [](AsyncWebServerRequest *request){
        // Verifica se o arquivo mqtt.html existe no SD
        if (SD.exists("/www/mqtt.html")) {
            // Enviamos o arquivo do SD passando o processor para renderizar templates (%WIFI_SSID%, etc.)
            request->send(SD, "/www/mqtt.html", "text/html", false, processor);
        } else {
            request->send(404, "text/plain", "Arquivo mqtt.html nao encontrado no SD");
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
    // Rota para obter a lista de dispositivos para ESP-NOW
    server.on("/espnow", HTTP_GET, [](AsyncWebServerRequest *request){
        int n = WiFi.scanComplete(); // Verifica o status do scan

        // Se o scan ainda não foi iniciado ou está em progresso
        if(n < 0) {
            if(n == -2) {
                // Inicia um novo scan em background (async = true)
                // show_hidden = true ajuda a encontrar dispositivos que não transmitem SSID
                WiFi.scanNetworks(true, true); 
            }
            // Retorna 202 para o JavaScript continuar tentando no loop 'while'
            request->send(202, "application/json", "{\"status\":\"scanning\"}");
            return;
        }

        // Se chegou aqui, o scan terminou (n >= 0)
        JsonDocument doc; 
        JsonArray root = doc.to<JsonArray>();

        for (int i = 0; i < n; ++i) {
            JsonObject item = root.add<JsonObject>();
            
            // Dados fundamentais para o ESP-NOW
            item["ssid"] = WiFi.SSID(i).length() > 0 ? WiFi.SSID(i) : "* Rede Oculta *";
            item["mac"]  = WiFi.BSSIDstr(i); // O endereço MAC que seu JS espera
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); // Retorna a lista completa com status 200
        WiFi.scanDelete(); // Importante: deleta o scan da memória para permitir que o próximo clique no botão reinicie o processo
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
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;

        // Se carregar o config e ele não for nulo
        if (accessSys.loadConfig(doc)) {
            
            // 1. Sincronização do estado físico dos Pinos
            if(doc["pins"].is<JsonArray>()){
                for (JsonObject p : doc["pins"].as<JsonArray>()) {
                    if (p.containsKey("pin")) {
                        p["state"] = digitalRead(p["pin"].as<int>());
                    }
                }
            } else {
                doc["pins"].to<JsonArray>(); // Garante [] se não existir
            }

            // 2. Verificação Real da Conexão MQTT (Hardware -> JSON)
            if (doc["mqtt"].is<JsonArray>()) {
                for (JsonObject p : doc["mqtt"].as<JsonArray>()) {
                    // p["active"].as<bool>() garante que o tipo seja tratado corretamente
                    if (p["active"].as<bool>() == true) {
                        // O status 'online' vem direto do PubSubClient via mqttService
                        p["online"] = mqttService.isConnected(); 
                    } else {
                        p["online"] = false;
                    }
                }
            } else {
                doc["mqtt"].to<JsonArray>();
            }

            String response;
            serializeJson(doc, response);
            request->send(200, "application/json", response);

        } else {

            // Fallback limpo: apenas o básico necessário
            request->send(200, "application/json", "{\"pins\":[],\"mqtt\":[]}");
       }
    });
    server.on("/config_mqtt", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, 
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        static std::vector<uint8_t> buffer;
        if (index == 0) { buffer.clear(); buffer.reserve(total); }
        buffer.insert(buffer.end(), data, data + len);

        if (index + len < total) return; 

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, buffer.data(), buffer.size());
        std::vector<uint8_t>().swap(buffer); // Libera RAM imediatamente

        if (error) return request->send(400);

        // 1. Geração de UUID consistente
        if (!doc.containsKey("uuid")) {
            uuid.generate();
            doc["uuid"] = (char*)uuid.toCharArray(); 
        }

        // 2. Validação de campo obrigatório
        if (doc.containsKey("broker")) {
            
            // EXECUÇÃO ÚNICA: Tenta salvar e verifica o retorno booleano
            if (accessSys.saveMqttFullConfig(doc.as<JsonObject>())) {
                
                // Se salvou, atualiza o serviço ativo imediatamente
                if (doc["active"] | false) {
                    mqttService.updateConfig(
                        doc["broker"] | "", 
                        doc["port"]   | 1883, 
                        doc["topic"]  | "", 
                        doc["user"]   | "", 
                        doc["passw"]  | "",
                        doc["qos"]    | 0,
                        doc["ssl"]    | false
                    );
                    mqttService.begin();
                    mqttService.forceUpdate();
                    Serial.println(F("Serviço MQTT atualizado: Perfil Ativo."));
                } else {
                    Serial.println(F("Perfil salvo em background (Inativo)."));
                }
                
                // Resposta JSON profissional de sucesso
                String responseUUID = doc["uuid"].as<String>();
                request->send(200, "application/json", "{\"status\":\"success\",\"uuid\":\"" + responseUUID + "\"}");
                
            } else {
                // Se cair aqui, ou o limite de 10 perfis estourou ou é duplicado
                request->send(409, "application/json", "{\"status\":\"error\",\"message\":\"Conflito: IP/Topico ja cadastrado ou limite atingido\"}");
            }
        } else {
            request->send(422, "application/json", "{\"status\":\"error\",\"message\":\"Falta o campo broker\"}");
        }
    });

    server.on("/set_active_mqtt", HTTP_PATCH, [](AsyncWebServerRequest *request){}, NULL, 
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        static std::vector<uint8_t> buffer;
        if (index == 0) { buffer.clear(); buffer.reserve(total); }
        buffer.insert(buffer.end(), data, data + len);

        if (index + len < total) return; 

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, buffer.data(), buffer.size());
        std::vector<uint8_t>().swap(buffer);

        if (error || !doc.containsKey("uuid")) return request->send(400, "application/json", "{\"error\":\"UUID obrigatorio\"}");

        const char* targetUuid = doc["uuid"];

        if (accessSys.toggleMqttActive(targetUuid)) {
            JsonDocument fullConfig;
            bool encontrouAtivo = false; // Flag crucial

            if (accessSys.loadConfig(fullConfig)) {
                for (JsonObject item : fullConfig["mqtt"].as<JsonArray>()) {
                    if (item["active"] == true) {
                        // Configura e conecta se houver um ativo
                        mqttService.updateConfig(
                            item["broker"] | "", 
                            item["port"]   | 1883, 
                            item["topic"]  | "", 
                            item["user"]   | "", 
                            item["passw"]  | "",
                            item["qos"]    | 0,
                            item["ssl"]    | false
                        );
                        mqttService.begin();
                        mqttService.forceUpdate();
                        encontrouAtivo = true;
                        break; 
                    }
                }
            }

            // Se após o loop NINGUÉM estiver ativo, desconectamos o hardware
            if (!encontrouAtivo) {
                Serial.println(F("[MQTT] Interface: Desativando conexão..."));
                mqttService.disconnect();
            }

            request->send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            request->send(404, "application/json", "{\"error\":\"UUID nao encontrado\"}");
        }
    });
    server.on("/delete_mqtt", HTTP_DELETE, [](AsyncWebServerRequest *request){}, NULL, 
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        static std::vector<uint8_t> buffer;
        if (index == 0) { buffer.clear(); buffer.reserve(total); }
        buffer.insert(buffer.end(), data, data + len);

        if (index + len < total) return; 

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, buffer.data(), buffer.size());
        std::vector<uint8_t>().swap(buffer);

        if (error || !doc.containsKey("uuid")) {
            return request->send(400, "application/json", "{\"error\":\"UUID obrigatorio\"}");
        }

        // 1. Chama a lógica de exclusão no sistema de arquivos (LittleFS)
        if (accessSys.deleteMqttProfile(doc["uuid"])) {
            
            // 2. Verifica se o perfil deletado era o que estava ativo no hardware
            JsonDocument fullConfig;
            bool restouAlgumAtivo = false;

            if (accessSys.loadConfig(fullConfig)) {
                if (fullConfig.containsKey("mqtt") && fullConfig["mqtt"].is<JsonArray>()) {
                    for (JsonObject item : fullConfig["mqtt"].as<JsonArray>()) {
                        if (item["active"] == true) {
                            restouAlgumAtivo = true;
                            break; // Ainda existe um perfil ativo configurado
                        }
                    }
                }
            }

            // 3. Se NÃO restou nenhum perfil ativo no arquivo, limpa o objeto e desliga a conexão
            if (!restouAlgumAtivo) {
                Serial.println(F("[MQTT] Perfil ativo deletado! Desconectando hardware e limpando objeto..."));
                
                mqttService.disable();     // Desabilita a verificação de tempo no loop do sketch
                mqttService.disconnect();  // Corta o socket TCP com o Broker imediatamente
                
                // Opcional: Reseta as strings internas para o estado padrão ("vazio")
                mqttService.updateConfig("", 1883, "", "", "", 0, false); 
            }

            request->send(200, "application/json", "{\"status\":\"deleted\"}");
        } else {
            request->send(404, "application/json", "{\"error\":\"UUID nao encontrado\"}");
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
