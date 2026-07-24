#include "MochiWebSocketClient.h"

// Ponteiro global oculto para ligar o callback estático à instância ativa
static MochiWebSocketClient* wsClientInstance = nullptr;

MochiWebSocketClient::MochiWebSocketClient(Animations* animationsPtr)
    : _animations(animationsPtr) {

    wsClientInstance = this; // Regista esta instância globalmente
}

void MochiWebSocketClient::begin(const char* host, int port, const char* url) {
    Serial.printf("[WS] A ligar ao servidor: ws://%s:%d%s\n", host, port, url);
    webSocket.begin(host, port, url);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);

    // Ativa o Heartbeat automático (PING a cada 5s)
    webSocket.enableHeartbeat(5000, 3000, 2); 
}

void MochiWebSocketClient::loop() {
    webSocket.loop();
}

void MochiWebSocketClient::verificarFluxoDados() {
    // Se não estiver conectado ou se o tempo sem pacotes estourar o timeout (servidor parou)
    unsigned long tempoSemDados = millis() - _lastPacketTime;
    
    if (!webSocket.isConnected() || (tempoSemDados > _dataTimeout)) {
        static unsigned long ultimoPrintAviso = 0;
        static bool alertaDisparadoNaTela = false;

        // 1. O cronômetro controla apenas o log do console para não travar a CPU
        if (millis() - ultimoPrintAviso > 3000) { 
            ultimoPrintAviso = millis();
            if (!webSocket.isConnected()) Serial.println("[WS STATUS] 📡 Desconectado do servidor. Rodando animação local...");
            else Serial.printf("[WS INFORMAÇÃO] ⏱️ Conectado, mas sem strings de animação há %lu ms!\n", tempoSemDados);
            
            if (_isReceivingData) {
                 Serial.println("[WS ALERTA] 🔴 O fluxo de dados que estava ativo caiu!");
                _isReceivingData = false;
            }
        }

        // 2. Garante que saímos do estado de recepção ativa
        _isReceivingData = false;

        // 3. A animação local roda solta aqui SEMPRE que o servidor estiver parado
        if (_animations) {
            Serial.println("[DISPLAY] Ativando modo de contingência visual (animationsLoop)...");
            _animations->animationsLoop(); 
        }
        
    } else {
        // 🟢 CASO CONTRÁRIO: O Servidor está ativo e enviando dados dentro do tempo limite!
        // Não chamamos o animationsLoop() aqui porque os frames estão vindo direto do 
        // método processarPayloadAnimacao() através do drawHexFrame().
        
        _isReceivingData = true; 
    }
}
// =========================================================================
// MÉTODO DE TRATAMENTO: Identifica e valida a String vinda do Worker
// =========================================================================
void MochiWebSocketClient::processarPayloadAnimacao(const char* payloadStr) {
    if (payloadStr == nullptr || strlen(payloadStr) == 0) {
        Serial.println("[WS PROCESSADOR] Erro: String vazia ou nula recebida.");
        return;
    }

    size_t tamanho = strlen(payloadStr);
    
    // 🔍 DEBUG 2: Printar o início real do texto enviado pelo Worker
    //Serial.printf("[WS PROCESSADOR] Primeiros 15 caracteres recebidos: \"%.15s\"\n", payloadStr);

    //Serial.printf("[WS PROCESSADOR] Pacote Identificado. Tamanho: %d bytes. ", tamanho);

    // Filtro de Segurança Básica: Verifica se possui caracteres Hex válidos (0-9, A-F)
    bool ehHexValido = true;
    for(size_t i = 0; i < min(tamanho, (size_t)10); i++) {
        char c = tolower(payloadStr[i]);
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '\n' || c == '\r')) {
            ehHexValido = false;
            break;
        }
    }

    if (ehHexValido) {
        //Serial.println("Tipo: Matriz Hex Bruta [Válida].");
        
        if (_animations) _animations->drawHexFrame((char*)payloadStr);

    } else {

        if (payloadStr[0] == '{') Serial.println("Tipo: Comando JSON detectado.");
        else Serial.printf("Tipo: Desconhecido ou Corrompido. Conteúdo inicial: %.10s\n", payloadStr);
    
    }
}

void MochiWebSocketClient::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    if (!wsClientInstance) return;

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Desconectado do Servidor de Animações!");
            wsClientInstance->_isReceivingData = false;
            
            if (wsClientInstance->_animations)  
                wsClientInstance->_animations->noworker();
            break;
            
        case WStype_CONNECTED:
            Serial.printf("[WS] Conectado com sucesso ao URL: %s\n", payload);
            wsClientInstance->_lastPacketTime = millis();
            break;
            
        case WStype_TEXT:
        case WStype_BIN:
            if (length > 0) {
                wsClientInstance->_lastPacketTime = millis();
                
                if (!wsClientInstance->_isReceivingData) {
                    Serial.println("[WS STATUS] 🟢 Fluxo de dados ativo com o Worker!");
                    wsClientInstance->_isReceivingData = true;
                }

                wsClientInstance->processarPayloadAnimacao((const char*)payload);
            }
            break;
            
        case WStype_ERROR:
            Serial.printf("[WS Erro] Falha crítica na transmissão de quadros! Código/Tamanho: %d\n", length);
            if (wsClientInstance->_animations) wsClientInstance->_animations->bugframe();
            break;

        default:
            break;
    }
}

// TESTES VIA CONSOLE
void MochiWebSocketClient::testarConexaoWS() {
    Serial.println("\n--- [CONSOLE] DIAGNÓSTICO DE CONECTIVIDADE WS ---");
    
    // Verifica se a conexão física está ativa no momento
    if (_isReceivingData) {
        Serial.println("Status: 🟢 Ativo e recebendo frames do Worker.");
    } else {
        Serial.println("Status: 🔴 Inativo ou sem dados trafegando.");
    }

    // Calcula há quanto tempo não chega nenhum frame
    unsigned long tempoSemDados = millis() - _lastPacketTime;
    Serial.printf("Tempo desde o último pacote: %.2f segundos\n", tempoSemDados / 1000.0);
    
    Serial.println("-------------------------------------------------");
}