#include "MochiWebSocketClient.h"

static MochiWebSocketClient* wsClientInstance = nullptr;

MochiWebSocketClient::MochiWebSocketClient(Animations* animationsPtr)
    : _animations(animationsPtr) {
    wsClientInstance = this;
}

void MochiWebSocketClient::begin(const char* host, int port, const char* url) {
    Serial.printf("[WS] Conectando a ws://%s:%d%s\n", host, port, url);
    webSocket.begin(host, port, url);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
    webSocket.enableHeartbeat(5000, 3000, 2); 
}

void MochiWebSocketClient::loop() {
    webSocket.loop();
}

void MochiWebSocketClient::verificarFluxoDados() {
    // Timeout ou desconexão ativa o fallback
    if (!webSocket.isConnected() || (millis() - _lastPacketTime > _dataTimeout)) {
        
        // Gatilho de queda: Limpa a tela apenas uma vez ao perder a conexão
        if (_isReceivingData) {
            Serial.println("[WS] Fluxo offline. Fallback ativado.");
            _isReceivingData = false;
            if (_animations) _animations->drawHexFrame(""); 
        }

        // 🎯 Garante a execução contínua da animação 'noworker' apenas se o Wi-Fi estiver ativo
        if (WiFi.status() == WL_CONNECTED && _animations) _animations->defaultlocal(); 
        
    } else {
        // Gatilho de restauração do servidor
        if (!_isReceivingData) {
             Serial.println("[WS] Fluxo online. Renderizando servidor.");
             _isReceivingData = true;
        }
    }
}

void MochiWebSocketClient::processarPayloadAnimacao(const char* payloadStr, size_t length) {
    if (!payloadStr) return;
    
    size_t tamanho = (length > 0) ? length : strlen(payloadStr);
    if (tamanho == 0) return;

    // Comando de limpeza
    if (tamanho == 5 && (strncmp(payloadStr, "CLEAR", 5) == 0 || strncmp(payloadStr, "clear", 5) == 0)) {
        if (_animations) _animations->drawHexFrame(""); 
        return;
    }

    // Filtro Hexadecimal rápido
    bool ehHexValido = true;
    for(size_t i = 0; i < min(tamanho, (size_t)10); i++) {
        char c = tolower(payloadStr[i]);
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '\n' || c == '\r')) {
            ehHexValido = false;
            break;
        }
    }

    if (ehHexValido && _animations) _animations->drawHexFrame(payloadStr);
}

void MochiWebSocketClient::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    if (!wsClientInstance) return;

    switch(type) {
        case WStype_DISCONNECTED:
            wsClientInstance->_isReceivingData = false;
            
            // 🎯 Dispara o estado imediato de 'noworker' se o Wi-Fi continuar online
            if (WiFi.status() == WL_CONNECTED && wsClientInstance->_animations) wsClientInstance->_animations->noworker();
            break;

        case WStype_CONNECTED:
            wsClientInstance->_lastPacketTime = millis();
            break;
            
        case WStype_TEXT:
        case WStype_BIN:
            if (length > 0 && payload != nullptr) {
                wsClientInstance->_lastPacketTime = millis();
                
                uint8_t tempChar = payload[length]; 
                payload[length] = '\0';             
                wsClientInstance->processarPayloadAnimacao((const char*)payload, length);
                payload[length] = tempChar;         
            }
            break;
            
        case WStype_ERROR:
            wsClientInstance->_isReceivingData = false;
            if (wsClientInstance->_animations) wsClientInstance->_animations->bugframe();
            break;

        default:
            break;
    }
}

void MochiWebSocketClient::testarConexaoWS() {
    Serial.println(_isReceivingData ? "[WS] Status: 🟢 Recebendo frames." : "[WS] Status: 🔴 Inativo.");
}