#include "AutoDiscoveryESP32.h"

AutoDiscoveryESP32::AutoDiscoveryESP32(const char* defaultEndpoint, uint8_t maxFailures)
    : m_endpointUrl(defaultEndpoint), 
      m_falhasConsecutivas(0), 
      m_maxFailures(maxFailures), 
      m_scanning(false),
      m_currentHostStep(0),
      m_startHost(0),
      m_endHost(0),
      m_lastScanStepMs(0),
      m_lastScanAttemptMs(0) {}

const char* AutoDiscoveryESP32::getEndpointUrl() const {
    return m_endpointUrl.c_str();
}

void AutoDiscoveryESP32::notifyTelemetryStatus(bool success) {
    if (success) {
        m_falhasConsecutivas = 0;
        m_scanning = false; // Cancela varredura se voltar a responder
    } else {
        if (!m_scanning) {
            m_falhasConsecutivas++;
            Serial.printf("[AutoDiscovery] Falha de telemetria registrada (%d/%d).\n", 
                          m_falhasConsecutivas, m_maxFailures);
        }
    }
}

void AutoDiscoveryESP32::prepareScanRange() {
    IPAddress localIP = WiFi.localIP();
    IPAddress subnet  = WiFi.subnetMask();

    uint32_t ipUint   = (uint32_t)localIP;
    uint32_t maskUint = (uint32_t)subnet;

    uint32_t networkUint   = ipUint & maskUint;
    uint32_t broadcastUint = networkUint | (~maskUint);

    m_startHost = ntohl(networkUint) + 1;
    m_endHost   = ntohl(broadcastUint) - 1;
    m_currentHostStep = m_startHost;
    m_localIP   = localIP;
}

bool AutoDiscoveryESP32::testSingleIP(IPAddress targetIP, String& outEndpointUrl) {
    if (targetIP == m_localIP) return false;

    HTTPClient http;
    // Connect Timeout restaurado para dar tempo ao TCP Handshake no Wi-Fi
    http.setConnectTimeout(300); 
    http.setTimeout(300);

    String targetUrl = "http://" + targetIP.toString() + "/api/autodiscovery";

    http.begin(targetUrl);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST("{\"host\":\"AUTO_DISCOVERY_TEST\"}");
    bool found = false;

    if (httpCode == HTTP_CODE_OK || httpCode == 200) {
        String payload = http.getString();
        if (payload.indexOf("\"status\":\"ok\"") != -1 || payload.indexOf("status: ok") != -1) {
            outEndpointUrl = "http://" + targetIP.toString() + "/api/telemetry";
            found = true;
        }
    }
    http.end();
    return found;
}

// Executado continuamente no loop() sem travar
void AutoDiscoveryESP32::tick() {
    // 1. Condição para iniciar a varredura
    if (!m_scanning && m_falhasConsecutivas >= m_maxFailures) {
        // Pausa de segurança de 30 segundos entre tentativas inteiras de scan
        if (millis() - m_lastScanAttemptMs >= 30000 || m_lastScanAttemptMs == 0) {
            if (WiFi.status() == WL_CONNECTED) {
                m_scanning = true;
                m_lastScanAttemptMs = millis();
                prepareScanRange();
                Serial.println("⚠️ [Autocura] Servidor indisponível. Iniciando busca não-bloqueante na sub-rede...");
            }
        }
    }

    // 2. Passo incremental da varredura (1 IP a cada 50ms)
    if (m_scanning) {
        if (WiFi.status() != WL_CONNECTED) {
            m_scanning = false;
            return;
        }

        if (millis() - m_lastScanStepMs >= 200) { 
            m_lastScanStepMs = millis();

            if (m_currentHostStep <= m_endHost) {
                IPAddress targetIP(htonl(m_currentHostStep));
                m_currentHostStep++;

                String discoveredEndpoint = "";
                if (testSingleIP(targetIP, discoveredEndpoint)) {
                    m_endpointUrl = discoveredEndpoint;
                    m_falhasConsecutivas = 0;
                    m_scanning = false;
                    Serial.printf("✅ [Autocura] Novo IP localizado! Endpoint: %s\n", m_endpointUrl.c_str());
                }
            } else {
                // Fim da sub-rede sem encontrar ninguém
                Serial.println("❌ [Autocura] Varredura completa. Ninguém respondeu.");
                m_scanning = false;
                m_falhasConsecutivas = 0; // Aguarda novas falhas antes de rescaniar
            }
        }
    }
}

bool AutoDiscoveryESP32::forceScan() {
    if (WiFi.status() != WL_CONNECTED) return false;
    m_falhasConsecutivas = m_maxFailures;
    m_lastScanAttemptMs = 0;
    tick();
    return true;
}