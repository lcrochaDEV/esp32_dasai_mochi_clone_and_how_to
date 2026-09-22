#include "TelemetryClient.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "AccessControl.h"
AccessControl accessSys; 

bool sendDeviceTelemetry(const char* destinationUrl) {
    // 1. Garante que há conexão de rede ativa
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Telemetria] Conexão Wi-Fi ausente. Abortando envio.");
        return false;
    }

    // 2. Constrói o Objeto exatamente no modelo esperado pelo Front-end / API
    JsonDocument doc;

    // Identificação e Conectividade
    doc["id"]              = WiFi.macAddress();
    doc["ip"]              = WiFi.localIP().toString();
    doc["mac"]             = WiFi.macAddress();
    doc["server"]          = "ESP32-Microcontroller";
    doc["host"]            = "ESP32-" + WiFi.macAddress().substring(9);

    // Tempo de Atividade (Uptime) e Epoch
    doc["datetime"]        = "2026-09-21 12:52:00"; // Pode ser integrado dinamicamente com NTP
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

    // Mapeamento proporcional da memória Heap (RAM) com tipos numéricos numéricos (uint32_t)
    uint32_t totalHeapKb = accessSys.total_ram_kb(); 
    uint32_t freeHeapKb  = accessSys.free_ram_kb();  
    uint32_t usedHeapKb  = (totalHeapKb > freeHeapKb) ? (totalHeapKb - freeHeapKb) : 0;

    doc["ram_pct"]         = (totalHeapKb > 0) ? ((usedHeapKb * 100) / totalHeapKb) : 0;
    doc["ram_total_mb"]    = totalHeapKb / 1024;
    doc["ram_used_mb"]     = usedHeapKb / 1024;

    // Mapeamento de memória Flash/Disco
    uint32_t flashTotalMb = accessSys.flash_size_mb();
    uint32_t sketchSizeKb = accessSys.sketch_size_kb();

    uint32_t flashTotalKb = flashTotalMb * 1024;
    doc["disk_pct"]        = (flashTotalKb > 0) ? ((sketchSizeKb * 100) / flashTotalKb) : 0;
    doc["disk_used_gb"]    = (float)sketchSizeKb / (1024.0 * 1024.0); 
    doc["disk_total_gb"]   = (float)flashTotalMb / 1024.0;

    // Identidade Visual e Status do Dispositivo
    doc["logo_url"]        = "https://cdn.shopify.com/shop-assets/shopify_brokers/dasaijp.myshopify.com/1760401281/logo_2.png?width=640";
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