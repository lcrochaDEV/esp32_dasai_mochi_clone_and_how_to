// FileSystemControl.h
#ifndef FILESYSTEMCONTROL_H
#define FILESYSTEMCONTROL_H

#include <Arduino.h>
#include <SD.h>          // Modificado de LittleFS para SD para a migração de hardware
#include <ArduinoJson.h>

// Definição de constantes - Atualizado para refletir a pasta padrão do SD
#define FILE_PATH "/data/config.json"

enum PinMode_t {
    MODE_INPUT = 0,
    MODE_OUTPUT = 1,
    MODE_INPUT_PULLUP = 2,
    MODE_INPUT_PULLDOWN = 3,
    MODE_KEEP = 99 // O nosso sentinela profissional
};

#define PIN_KEEP -1
#define LINK_KEEP -99

struct PinLink {
    int master;
    int slave;
};

class FileSystemControl {
  private:
    String _ssid;
    String _password;
    void _addPinConfig(int pin = 0, PinMode_t mode = MODE_OUTPUT, int state = 0, int level = 0, int vincularpin = -1);

  public:
    // Construtor
    FileSystemControl(const char* ssid = nullptr, const char* password = nullptr);

    // Inicializa o sistema de arquivos no SD
    bool begin();
    void syncHardwareWithDisk(); // Recupera dados e Reconfigura os Pinos
    
    // Métodos de manipulação de JSON
    void saveConfig(JsonDocument& doc);
    bool loadConfig(JsonDocument& doc);
    void fillJson(JsonDocument& doc, const char* ssid, const char* pass);
    
    // Inclusão de dados no json  
    void setPinMode(int pin, PinMode_t mode);
    void setPinLevel(int pin, int level);
    void setPinState(int pin, int state);
    void setPinLink(int pin, int vincularpin);
    
    // Método principal para persistir as credenciais da classe
    bool credentials();
    void returnObjectData();
    int getLinkedPin(int originPin);
    void runMirroring();
    void factoryReset(); // DELETA ARQUIVO DE CONFIGURAÇÕES do SD   
    
    // MQTT CONFIG (Ajustado o nome para bater com a implementação do .cpp)
    bool saveMqttFullConfig(JsonObject newConfig); 
    bool toggleMqttActive(const char* uuid);
    bool deleteMqttProfile(const char* uuid);
};

#endif