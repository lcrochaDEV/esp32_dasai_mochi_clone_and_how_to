#ifndef ESP_MQTT_H
#define ESP_MQTT_H

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClientSecure.h> // Incluído explicitamente para o WiFiClientSecure no ESP32
#endif
#include <PubSubClient.h>

void processarMensagemMqtt(char* topic, byte* payload, unsigned int length);

class EspMqtt {
  private:
    WiFiClient espClient;
    WiFiClientSecure secureClient; // Cliente para conexões SSL
    PubSubClient client;
    bool _useSsl; // Armazena se deve usar SSL ou não
    
    // TRECHO MODIFICADO: Alterado de const char* para String para clonar os dados na RAM
    String _server;
    int _port;
    String _topic;
    String _clientId;
    String _user;
    String _passw;
    int _qos;
    unsigned long lastReconnectAttempt;
    
    // --- NOVAS VARIÁVEIS ENCAPSULADAS DE TEMPO ---
    bool _enabled;                  // Controla se o loop deve processar
    unsigned long _lastUpdateTime;   // Armazena o último timestamp do millis
    const unsigned long _updateInterval = 10; // Executa rigidamente a cada 10ms

  public:
    // Construtor: Inicializa com os dados básicos
    EspMqtt(const char* server = nullptr, int port = 1883, const char* clientId = "ESP32_Nexus");

    // Inicialização: Define o tópico e a função de callback para mensagens recebidas
    void begin();

    // Atualização Dinâmica: Essencial para o novo endpoint /config_mqtt
    // Permite trocar o Broker e Tópico sem resetar o ESP32
    void updateConfig(const char* newServer, int newPort, const char* newTopic, const char* newUser, const char* newPassw, int newQos = 0, bool useSsl = false);
    void reconnect();
    
    // Loop principal: Gerencia reconexão não-bloqueante com millis interno
    void update();
    void forceUpdate();
    
    // Métodos de Publicação
    bool publish(const char* payload);
    bool publishToTopic(const char* customTopic, const char* payload);

    // Getters adaptados para retornar const char* a partir das novas Strings internas (.c_str())
    const char* getServer() { return _server.c_str(); }
    const char* getTopic() { return _topic.c_str(); }
    bool isConnected() { return client.connected(); }
    void disconnect();

    // --- NOVOS MÉTODOS DE CONTROLE DO CICLO ASSÍNCRONO ---
    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
    bool isEnabled() { return _enabled; }
};

#endif