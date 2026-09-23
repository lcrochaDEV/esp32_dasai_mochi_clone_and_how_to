#include "AutoDiscoveryESP32.h"

AutoDiscoveryESP32::AutoDiscoveryESP32(const char* defaultEndpoint, uint8_t maxFailures)
    : m_endpointUrl(defaultEndpoint), 
      m_falhasConsecutivas(0), 
      m_maxFailures(maxFailures), 
      m_scanning(false) {}

const char* AutoDiscoveryESP32::getEndpointUrl() const {
    return m_endpointUrl.c_str();
}

void AutoDiscoveryESP32::notifyTelemetryStatus(bool success) {
    if (success) {
        m_falhasConsecutivas = 0; // Reseta se o envio foi bem-sucedido
    } else {
        m_falhasConsecutivas++;
        Serial.printf("[AutoDiscovery] Falha registrada (%d/%d).\n", m_falhasConsecutivas, m_maxFailures);
    }
}

// Executado continuadamente dentro do loop()
void AutoDiscoveryESP32::tick() {
    // Se o número de falhas atingiu o limite por conta de alteração de IP no DHCP
    if (m_falhasConsecutivas >= m_maxFailures && !m_scanning) {
        if (WiFi.status() != WL_CONNECTED) return;

        m_scanning = true;
        Serial.println("⚠️ [Autocura] Servidor indisponível. Calculando sub-rede e buscando novo IP...");

        String newEndpoint = "";
        if (executeSubnetScan(newEndpoint)) {
            m_endpointUrl = newEndpoint;
            m_falhasConsecutivas = 0;
            Serial.printf("✅ [Autocura] Novo IP localizado! Endpoint: %s\n", m_endpointUrl.c_str());
        } else {
            Serial.println("❌ [Autocura] Servidor não localizado nesta varredura.");
            // Reseta parcialmente para não travar a CPU tentando sem parar a cada milissegundo
            m_falhasConsecutivas = m_maxFailures - 1; 
        }

        m_scanning = false;
    }
}

bool AutoDiscoveryESP32::executeSubnetScan(String& outEndpointUrl) {
    IPAddress localIP = WiFi.localIP();
    IPAddress subnet  = WiFi.subnetMask();

    uint32_t ipUint   = (uint32_t)localIP;
    uint32_t maskUint = (uint32_t)subnet;

    uint32_t networkUint   = ipUint & maskUint;
    uint32_t broadcastUint = networkUint | (~maskUint);

    uint32_t startHost = ntohl(networkUint) + 1;
    uint32_t endHost   = ntohl(broadcastUint) - 1;

    HTTPClient http;
    http.setTimeout(150);

    for (uint32_t host = startHost; host <= endHost; ++host) {
        IPAddress targetIP(htonl(host));
        if (targetIP == localIP) continue;

        String targetUrl = "http://" + targetIP.toString() + "/api/autodiscovery";

        http.begin(targetUrl);
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST("{\"host\":\"AUTO_DISCOVERY_TEST\"}");

        if (httpCode == HTTP_CODE_OK || httpCode == 200) {
            String payload = http.getString();
            if (payload.indexOf("\"status\":\"ok\"") != -1 || payload.indexOf("status: ok") != -1) {
                outEndpointUrl = "http://" + targetIP.toString() + "/api/telemetry";
                http.end();
                return true;
            }
        }
        http.end();
        yield(); // Libera o Watchdog do ESP32
    }
    return false;
}