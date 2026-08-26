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
    void processarPayloadAnimacao(const char* payloadStr, size_t length = 0);
    void testarConexaoWS();

  private:
    WebSocketsClient webSocket;
    Animations* _animations;

    unsigned long _lastPacketTime = 0;
    const unsigned long _dataTimeout = 5000;
    bool _isReceivingData = false;

    static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};

#endif