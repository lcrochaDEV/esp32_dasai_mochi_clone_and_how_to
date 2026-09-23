#include "TelemetryClient.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "AccessControl.h"
// Utiliza a instância global já existente para evitar duplicação de memória
AccessControl accessSys; 

bool sendDeviceTelemetry(const char* destinationUrl) {
    // 1. Garante que há conexão de rede ativa
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Telemetria] Conexão Wi-Fi ausente. Abortando envio.");
        return false;
    }

    // 2. Constrói o Objeto exatamente no modelo esperado pelo Front-end / API
    JsonDocument doc;

    // Obtém os bytes brutos do MAC do hardware (zero fragmentação no Heap)
    uint8_t macBytes[6];
    WiFi.macAddress(macBytes);

    // Formata o ID limpo sem os dois pontos ':' (Exemplo: "30C6F78490AC")
    char cleanId[13];
    snprintf(cleanId, sizeof(cleanId), "%02X%02X%02X%02X%02X%02X", 
             macBytes[0], macBytes[1], macBytes[2], macBytes[3], macBytes[4], macBytes[5]);

    // Identificação e Conectividade
    doc["id"]              = cleanId;                   // ID sem os dois pontos ':'
    doc["ip"]              = WiFi.localIP().toString();
    doc["mac"]             = WiFi.macAddress();        // Formato padrão com separadores "AA:BB:CC:DD:EE:FF"
    doc["server"]          = "ESP32-Microcontroller";
    doc["host"]            = String("ESP32-") + &cleanId[6];

    // Tempo de Atividade (Uptime) e Epoch
    doc["datetime"]        = "2026-09-21 12:52:00"; // Pode ser integrado dinamicamente com NTP[cite: 12]
    doc["epoch_timestamp"] = millis() / 1000;
    
    unsigned long totalSeconds = millis() / 1000;
    int days    = totalSeconds / 86400;
    int hours   = (totalSeconds % 86400) / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    
    char uptimeStr[32];
    snprintf(uptimeStr, sizeof(uptimeStr), "%dd %dh %dm", days, hours, minutes);
    doc["uptime"] = uptimeStr;

    // Dados de Hardware e Sistema
    doc["os_name"]         = "FreeRTOS / ESP-IDF";
    doc["kernel"]          = accessSys.modelBoardESP(); // ex: ESP32
    doc["temp"]            = 42.5; // Métricas de hardware
    doc["cpu_load"]        = 1.20;
    // -------------------------------------------------------------------------
    // Mapeamento de Memória RAM - DADOS BRUTOS (KB)
    // -------------------------------------------------------------------------
    // Mapeamento de memória Heap (RAM) em ponto flutuante (força decimal e impede que o valor vire 0)
    uint32_t totalHeapKb = accessSys.total_ram_kb(); 
    uint32_t freeHeapKb  = accessSys.free_ram_kb();  
    uint32_t usedHeapKb  = (totalHeapKb > freeHeapKb) ? (totalHeapKb - freeHeapKb) : 0;

    doc["ram_pct"]         = (totalHeapKb > 0) ? ((usedHeapKb * 100) / totalHeapKb) : 0;
    doc["ram_total_kb"]    = totalHeapKb; // 1024.0; // Envia ex: 0.31 MB
    doc["ram_used_kb"]     = usedHeapKb; // 1024.0;  // Envia ex: 0.15 MB

    // -------------------------------------------------------------------------
    // Mapeamento de Memória Flash / Disco - DADOS BRUTOS (KB)
    // -------------------------------------------------------------------------
    uint32_t flashTotalMb = accessSys.flash_size_mb();  // Exemplo: 4 MB
    uint32_t sketchSizeKb = accessSys.sketch_size_kb(); // Exemplo: 980 KB

    // Converte Flash Total de MB para KB (Multiplicação por 1024)
    uint32_t flashTotalKb = flashTotalMb * 1024;        // Exemplo: 4096 KB

    // Percentual de uso do Sketch em relação à Flash total
    doc["disk_pct"]      = (flashTotalKb > 0) ? ((sketchSizeKb * 100) / flashTotalKb) : 0;
    
    // Envia os inteiros brutos em KB sem divisões extras
    doc["disk_used_kb"]  = sketchSizeKb;  // Envia ex: 980 (KB puros)
    doc["disk_total_kb"] = flashTotalKb; // Envia ex: 4096 (KB puros)

    // Identidade Visual e Status do Dispositivo
    doc["logo_url"]        = "https://dasai.com.au/cdn/shop/files/logo_2.png?v=1746165958&width=4370";
    
    doc["online"]          = true;

    // 3. Serializa o JSON para string
    String payload;
    serializeJson(doc, payload);

    // 4. Executa o disparo HTTP POST simples para a API
    HTTPClient http;
    http.begin(destinationUrl);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(3000); // Define limite máximo de espera de 3 segundos

    int httpCode = http.POST(payload);
    bool success = false;

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == 201) {
            Serial.printf("[Telemetria] Enviada com sucesso! Código: %d\n", httpCode);
            success = true;
        } else {
            Serial.printf("[Telemetria] Servidor recusou a requisição. Código: %d\n", httpCode);
        }
    } else {
        Serial.printf("[Telemetria] Falha de comunicação HTTP: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Fecha a socket e libera a memória RAM usada na conexão
    return success;
}