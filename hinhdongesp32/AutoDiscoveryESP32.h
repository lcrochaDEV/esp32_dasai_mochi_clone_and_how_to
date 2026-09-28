#ifndef AUTO_DISCOVERY_ESP32_HPP
#define AUTO_DISCOVERY_ESP32_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

class AutoDiscoveryESP32 {
public:
    explicit AutoDiscoveryESP32(const char* defaultEndpoint = "http://192.168.1.6/api/telemetry", 
                                uint8_t maxFailures = 3);

    void tick();
    void notifyTelemetryStatus(bool success);
    const char* getEndpointUrl() const;
    bool forceScan();

private:
    String m_endpointUrl;
    uint8_t m_falhasConsecutivas;
    uint8_t m_maxFailures;
    bool m_scanning;

    // Variáveis de estado do Scan Incremental
    uint32_t m_currentHostStep;
    uint32_t m_startHost;
    uint32_t m_endHost;
    IPAddress m_localIP;
    unsigned long m_lastScanStepMs;
    unsigned long m_lastScanAttemptMs; // Pausa entre varreduras completas falhas

    void prepareScanRange();
    bool testSingleIP(IPAddress targetIP, String& outEndpointUrl);
};

#endif // AUTO_DISCOVERY_ESP32_HPP