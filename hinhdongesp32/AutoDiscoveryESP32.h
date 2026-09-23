#ifndef AUTO_DISCOVERY_ESP32_HPP
#define AUTO_DISCOVERY_ESP32_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

class AutoDiscoveryESP32 {
public:
    // Construtor aceita a URL inicial de fallback e o limite de falhas
    explicit AutoDiscoveryESP32(const char* defaultEndpoint = "http://192.168.1.6/api/telemetry", 
                                uint8_t maxFailures = 3);

    // Método único a ser chamado livremente no loop()
    void tick();

    // Notifica a classe sobre o resultado da última telemetria
    void notifyTelemetryStatus(bool success);

    // Retorna a URL atualizada para envio da telemetria
    const char* getEndpointUrl() const;

    // Força uma busca manual na rede se necessário
    bool forceScan();

private:
    String m_endpointUrl;
    uint8_t m_falhasConsecutivas;
    uint8_t m_maxFailures;
    bool m_scanning;

    bool executeSubnetScan(String& outEndpointUrl);
};

#endif // AUTO_DISCOVERY_ESP32_HPP