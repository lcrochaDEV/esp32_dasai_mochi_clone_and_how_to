// EspMqtt.cpp
#include "HardwareSerial.h"
#include "EspMqtt.h"
#include <ArduinoJson.h>

void processarMensagemMqtt(char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        Serial.print(F("Erro ao ler JSON: "));
        return;
    }

    // Verifica se o JSON tem as chaves necessárias
    if (doc.containsKey("pino") && doc.containsKey("state")) {
        int pino = doc["pino"];
        const char* status = doc["state"];

        // Configura o pino como saída caso não tenha sido feito
        pinMode(pino, OUTPUT);

        if (strcmp(status, "ON") == 0) {
            digitalWrite(pino, HIGH);
        } else {
            digitalWrite(pino, LOW);
        }

        // Print formatado para conferência
        Serial.print("Comando recebido -> Pino: ");
        Serial.print(pino);
        Serial.print(" | Estado: ");
        Serial.println(status);
    }
}

// TRECHO MODIFICADO: Construtor adaptado para inicializar com as novas variáveis do header
EspMqtt::EspMqtt(const char* server, int port, const char* clientId) 
    : client(espClient), _server(server ? server : ""), _port(port), _clientId(clientId ? clientId : "ESP32_Nexus"), 
      _user(""), _passw(""), _qos(0), _useSsl(false),
      _enabled(false), _lastUpdateTime(0) { 
    lastReconnectAttempt = 0;
}

void EspMqtt::begin() {
    // TRECHO MODIFICADO: Uso do .length() em vez de comparação direta de ponteiro const char*
    if (_server.length() > 0) {
        client.setServer(_server.c_str(), _port);
        client.setCallback(processarMensagemMqtt);
        Serial.printf("MQTT Iniciado para o servidor: %s\n", _server.c_str());
    } else {
        Serial.println("Erro: Tentativa de begin sem servidor configurado!");
    }
}

// TRECHO MODIFICADO: Ingestão de dados copiando as strings voláteis para instâncias fixas de String
void EspMqtt::updateConfig(const char* newServer, int newPort, const char* newTopic, 
                           const char* newUser, const char* newPassw, int newQos, bool useSsl) {
    _server = newServer ? newServer : "";
    _port = newPort;
    _topic = newTopic ? newTopic : "";
    _user = newUser ? newUser : "";
    _passw = newPassw ? newPassw : "";
    _qos = newQos;
    _useSsl = useSsl;

    // Lógica para alternar o cliente de rede
    if (_useSsl) {
        // No ESP32, setInsecure() permite conectar via SSL sem validar o certificado CA
        secureClient.setInsecure(); 
        client.setClient(secureClient);
    } else {
        client.setClient(espClient);
    }

    if (client.connected()) client.disconnect();
    
    // TRECHO MODIFICADO: Convertendo a String interna de volta para const char* exigido pela biblioteca
    client.setServer(_server.c_str(), _port);
    Serial.printf("Configuração MQTT Atualizada. SSL: %s\n", _useSsl ? "Sim" : "Não");
}

// TRECHO MODIFICADO: Adaptado para extrair e ler as Strings nativas persistidas (.c_str() e .length())
void EspMqtt::reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Aguardando WiFi para conectar MQTT...");
        return;
    }
    Serial.printf("Tentando MQTT em %s:%d...\n", _server.c_str(), _port);
    
    // Tenta conectar com usuário e senha validando a String clonada
    bool connected = (_user.length() > 0) ? 
        client.connect(_clientId.c_str(), _user.c_str(), _passw.c_str()) : 
        client.connect(_clientId.c_str());

    if (connected) {
        Serial.println("MQTT Conectado!");
        client.subscribe(_topic.c_str(), _qos);
    } else {
        Serial.printf("Falha rc=%d. Tentando em 5s\n", client.state());
    }
}

// TRECHO MODIFICADO: Inclusão do throttle de tempo (10ms) e trava de segurança assíncrona
void EspMqtt::update() {
    // Se o front-end desativou ou se está limpando a fila, sai instantaneamente sem gastar CPU
    if (!_enabled) return;

    unsigned long now = millis();
    // Throttle rígido de tempo interno
    if (now - _lastUpdateTime >= _updateInterval) {
        _lastUpdateTime = now;

        if (!client.connected()) {
            // Mantém a lógica de tentativa de reconexão a cada 5 segundos de forma não-bloqueante
            if (now - lastReconnectAttempt > 5000) {
                lastReconnectAttempt = now;
                reconnect();
            }
        } else {
            // Mantém o processamento interno e pings de Keep-Alive da biblioteca
            client.loop();
        }
    }
}

void EspMqtt::forceUpdate() { lastReconnectAttempt = 0; }

// TRECHO MODIFICADO: Uso do .c_str() para ler a String segura de tópico
bool EspMqtt::publish(const char* payload) {
    if (client.connected()) return client.publish(_topic.c_str(), payload);
    return false;
}

bool EspMqtt::publishToTopic(const char* customTopic, const char* payload) {
    if (client.connected()) return client.publish(customTopic, payload);
    return false;
}

void EspMqtt::disconnect() {
    if (client.connected()) {
        client.disconnect();
        Serial.println(F("MQTT Desconectado manualmente via Interface."));
    }
    lastReconnectAttempt = millis();
}