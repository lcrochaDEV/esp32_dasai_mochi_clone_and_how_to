#include "HardwareSerial.h"

// FileSystemControl.cpp
#include "FileSystemControl.h"
#include <SD.h> // Trocado de LittleFS para a biblioteca nativa do SD

FileSystemControl::FileSystemControl(const char* ssid, const char* password)
     : _ssid(ssid ? ssid : ""), _password(password ? password : "") {
  }

/**
 * @brief Prepara o diretório base no Cartão SD
 */
bool FileSystemControl::begin() {
    Serial.println("\n--- SISTEMA DE ARQUIVOS (CARTÃO SD) ---");
    
    // Como o SD.begin() já é gerenciado e startado pela classe SDData,
    // apenas garantimos que a pasta pai "/data" exista antes de operar.
    if (!SD.exists("/data")) {
        if (SD.mkdir("/data")) {
            Serial.println("Pasta '/data' criada com sucesso no SD.");
        } else {
            Serial.println("FALHA CRÍTICA: Não foi possível criar a pasta '/data' no SD.");
            return false;
        }
    }

    Serial.println("Cartão SD pronto para armazenamento de configurações!");
    syncHardwareWithDisk();
    return true;
}

/**
 * @brief Garante que os estados físicos dos pinos correspondam ao JSON no boot.
 */
void FileSystemControl::syncHardwareWithDisk() {
    JsonDocument doc;
    if (!loadConfig(doc) || !doc["pins"].is<JsonArray>()) return;

    for (JsonObject p : doc["pins"].as<JsonArray>()) {
        int pin   = p["pin"];
        int mode  = p["mode"];
        int level = p["level"];
        int state = p["state"]; // Adicionado o estado salvo

        if (mode != MODE_KEEP) {
            #if defined(ESP8266)
            if (mode == MODE_INPUT_PULLDOWN && pin == 16) pinMode(pin, INPUT_PULLDOWN_16);
            else pinMode(pin, (uint8_t)mode);
            #else
            pinMode(pin, (uint8_t)mode);
            #endif
        }

        if (mode == MODE_OUTPUT) {
            // Se o state for válido (não for PIN_KEEP), usamos ele, senão usamos o level
            int finalState = (level != PIN_KEEP) ? level : state;
            digitalWrite(pin, finalState);
        }
    }
    Serial.println("Hardware sincronizado (Mode/Level/State).");
}

/**
 * @brief Salva o JSON da memória para o arquivo no Cartão SD.
 */
void FileSystemControl::saveConfig(JsonDocument& doc) {
    // FILE_WRITE limpa e sobrescreve o arquivo no SD de forma limpa
    File file = SD.open(FILE_PATH, FILE_WRITE);
    if (!file) { 
        Serial.printf("Erro ao abrir arquivo '%s' para escrita no SD\n", FILE_PATH); 
        return; 
    }
    
    if (serializeJson(doc, file) == 0) {
        Serial.println("Falha ao escrever JSON no arquivo do SD");
    }

    file.close();
    delay(50);
    Serial.println("Configuração salva com sucesso no SD!");
}

/**
 * @brief Carrega o JSON do arquivo no Cartão SD para a memória.
 */
bool FileSystemControl::loadConfig(JsonDocument& doc) {
    if (!SD.exists(FILE_PATH)) {
        doc.to<JsonObject>();
        Serial.printf("Arquivo '%s' não existe no SD.\n", FILE_PATH);
        return false;
    }

    File file = SD.open(FILE_PATH, FILE_READ);
    if (!file) {
        doc.to<JsonObject>();
        Serial.println("Falha ao abrir arquivo para leitura no SD"); 
        return false; 
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();
  
    if (error) {
        Serial.print("Erro no JSON lido do SD: ");
        doc.to<JsonObject>();
        Serial.println(error.c_str());
        return false;
    }
    
    return true;
}

// Corrigido: Agora o nome coincide com o header e o uso interno
void FileSystemControl::fillJson(JsonDocument& doc, const char* ssid, const char* pass) {
    doc["ssid"] = ssid;
    doc["pass"] = pass;
}

bool FileSystemControl::credentials() {
    JsonDocument doc; 

    loadConfig(doc); // Carrega Objeto
    // Usa as variáveis privadas da classe
    // Verifica se houve mudança real para poupar ciclos de gravação no cartão SD
    if (doc["ssid"] == _ssid && doc["pass"] == _password) {
        Serial.println("Credenciais idênticas às já salvas. Ignorando escrita.");
        return true;
    }
    fillJson(doc, _ssid.c_str(), _password.c_str());
    saveConfig(doc);
    return true;
}

void FileSystemControl::returnObjectData() {
    // 1. Criamos o "contêiner" para o objeto literal
    JsonDocument config; 

    // 2. Passamos para a classe preencher
    if(loadConfig(config)) {
        Serial.println("Objeto Literal Recebido do SD:");
        
        // 3. Imprimindo o objeto literal formatado
        serializeJsonPretty(config, Serial);
        Serial.println();

    } else {
        Serial.println("Arquivo vazio ou inexistente no SD.");
    }
}

void FileSystemControl::_addPinConfig(int pin, PinMode_t mode, int state, int level, int vincularpin) {
    JsonDocument doc;
    loadConfig(doc);

    JsonArray pins = doc["pins"].is<JsonArray>() ? 
                     doc["pins"].as<JsonArray>() : 
                     doc["pins"].to<JsonArray>();

    JsonObject target;
    bool found = false;

    for (JsonObject p : pins) {
        if (p["pin"] == pin) {
            target = p;
            found = true;
            break;
        }
    }

    if (!found) {
        target = pins.add<JsonObject>();
        target["pin"] = pin;
        target["mode"] = (int)MODE_OUTPUT; 
        target["state"] = 0;
        target["level"] = 0;
        target["vincularpin"] = -1; // Valor padrão: sem vínculo
    }

    // A lógica profissional de "Patch" (Só altera o que foi solicitado)
    if (mode != MODE_KEEP)  target["mode"] = (int)mode;
    if (state != PIN_KEEP)  target["state"] = state;
    if (level != PIN_KEEP)  target["level"] = level;
    if (vincularpin != LINK_KEEP) target["vincularpin"] = vincularpin; // Grava o novo vínculo

    saveConfig(doc);
}

void FileSystemControl::setPinMode(int pin, PinMode_t mode) {
    _addPinConfig(pin, mode, PIN_KEEP, PIN_KEEP, LINK_KEEP);
}

void FileSystemControl::setPinLevel(int pin, int level) {
    _addPinConfig(pin, MODE_KEEP, PIN_KEEP, level, LINK_KEEP);
}

void FileSystemControl::setPinState(int pin, int state) {
    _addPinConfig(pin, MODE_KEEP, state, PIN_KEEP, LINK_KEEP);
}

/**
 * @brief Define o vínculo de espelhamento entre dois pinos no JSON.
 */
void FileSystemControl::setPinLink(int pin, int vincularpin) {
    _addPinConfig(pin, MODE_KEEP, PIN_KEEP, PIN_KEEP, vincularpin);
}

/**
 * @brief Busca no JSON qual pino de saída está vinculado a um pino de origem.
 */
int FileSystemControl::getLinkedPin(int originPin) {
    JsonDocument doc;
    if (!loadConfig(doc)) return -1;

    if (doc["pins"].is<JsonArray>()) {
        JsonArray pins = doc["pins"].as<JsonArray>();
        for (JsonObject p : pins) {
            if (p.containsKey("vincularpin") && p["vincularpin"].as<int>() == originPin) {
                return p["pin"].as<int>();
            }
        }
    }
    return -1; 
}

void FileSystemControl::runMirroring() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 50) return; 
    lastCheck = millis();

    JsonDocument doc;
    if (!loadConfig(doc) || !doc["pins"].is<JsonArray>()) return;
    JsonArray pins = doc["pins"].as<JsonArray>();

    bool mudancaDetectada = false;

    for (JsonObject pinoEscravo : pins) {
        int vPin = pinoEscravo.containsKey("vincularpin") ? pinoEscravo["vincularpin"].as<int>() : -1;

        if (vPin != -1) {
            for (JsonObject pinoMestre : pins) {
                if (pinoMestre["pin"] == vPin) {
                    int estadoMestre = pinoMestre["state"];
                    int estadoAtualEscravo = pinoEscravo["state"];

                    if (estadoAtualEscravo != estadoMestre) {
                        pinoEscravo["state"] = estadoMestre;
                        mudancaDetectada = true; 
                    }

                    digitalWrite(pinoEscravo["pin"].as<int>(), estadoMestre);
                    break; 
                }
            }
        }
    }

    if (mudancaDetectada) {
        saveConfig(doc);
        Serial.println("Mirroring: Estados sincronizados no JSON e salvos no SD.");
    }
}

// DELETA ARQUIVO DE CONFIGURAÇÕES DO CARTÃO SD
void FileSystemControl::factoryReset() {
    if (SD.exists(FILE_PATH)) {
        SD.remove(FILE_PATH);
        Serial.println("Arquivo de configuração removido do SD! Resetando...");
        ESP.restart(); 
    }
}

//MQTT SAVE JSON NO CARTÃO SD
bool FileSystemControl::saveMqttFullConfig(JsonObject newConfig) { // Ajustado o nome do método original
    JsonDocument doc;
    loadConfig(doc);

    JsonArray mqttArray;
    if (doc["mqtt"].is<JsonArray>()) {
        mqttArray = doc["mqtt"].as<JsonArray>();
    } else {
        mqttArray = doc["mqtt"].to<JsonArray>();
    }

    const char* newBroker = newConfig["broker"] | "";
    const char* newTopic  = newConfig["topic"]  | "";

    for (JsonObject existingMqtt : mqttArray) {
        if (strcmp(newBroker, existingMqtt["broker"] | "") == 0 && 
            strcmp(newTopic, existingMqtt["topic"] | "") == 0) {
            return false; // Duplicado
        }
    }

    bool isNewActive = newConfig["active"] | false;
    if (isNewActive) {
        for (JsonObject p : mqttArray) {
            p["active"] = false;
        }
    }

    if (mqttArray.size() >= 10) return false;
    mqttArray.add(newConfig);

    // Persistência usando FILE_WRITE (que limpa e reescreve)
    File file = SD.open(FILE_PATH, FILE_WRITE);
    if (file) {
        serializeJson(doc, file);
        file.close();
        return true;
    }
    return false;
}

bool FileSystemControl::toggleMqttActive(const char* uuid) {
    JsonDocument doc;
    if (!loadConfig(doc)) return false;
    if (!doc["mqtt"].is<JsonArray>()) return false;

    JsonArray mqttArray = doc["mqtt"].as<JsonArray>();
    bool found = false;

    for (JsonObject p : mqttArray) {
        const char* currentUuid = p["uuid"] | "";
        if (strcmp(currentUuid, uuid) == 0) {
            p["active"] = true;  
            found = true;
        } else {
            p["active"] = false; 
        }
    }

    if (found) {
        File file = SD.open(FILE_PATH, FILE_WRITE);
        if (file) {
            serializeJson(doc, file);
            file.close();
            return true;
        }
    }
    return false;
}

bool FileSystemControl::deleteMqttProfile(const char* uuid) {
    JsonDocument doc;
    if (!loadConfig(doc)) return false;
    if (!doc["mqtt"].is<JsonArray>()) return false;

    JsonArray mqttArray = doc["mqtt"].as<JsonArray>();
    bool removed = false;

    for (size_t i = 0; i < mqttArray.size(); i++) {
        if (strcmp(mqttArray[i]["uuid"] | "", uuid) == 0) {
            mqttArray.remove(i);
            removed = true;
            break; 
        }
    }

    if (removed) {
        File file = SD.open(FILE_PATH, FILE_WRITE);
        if (file) {
            serializeJson(doc, file);
            file.close();
            return true;
        }
    }
    return false;
}