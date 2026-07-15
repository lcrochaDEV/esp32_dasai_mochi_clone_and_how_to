#ifndef MOCHI_WEBSOCKET_CLIENT_H
#define MOCHI_WEBSOCKET_CLIENT_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include "Animations.h"

class MochiWebSocketClient {
  public:
    MochiWebSocketClient(Animations* animationsPtr);
    void begin(const char* host, int port, const char* url = "/ws/animations");
    void loop();
    void verificarFluxoDados();
    void processarPayloadAnimacao(const char* payloadStr);
    void testarConexaoWS();
  private:
    WebSocketsClient webSocket;
    Animations* _animations;

    // Controlo simplificado do Watchdog de dados
    unsigned long _lastPacketTime = 0;
    const unsigned long _dataTimeout = 5000; // 5 segundos
    bool _isReceivingData = false;

    // O callback obrigatório da biblioteca
    static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};

#endif