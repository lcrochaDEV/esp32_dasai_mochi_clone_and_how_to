#ifndef TELEMETRY_CLIENT_H
#define TELEMETRY_CLIENT_H

#include <Arduino.h>

/**
 * @brief Envia os dados de telemetria do microcontrolador para o endpoint HTTP.
 * @param destinationUrl URL do servidor de destino (ex: "http://192.168.1.6/api/telemetry")
 * @return true em caso de sucesso (HTTP 200/201), false em caso de erro/falha de envio.
 */
bool sendDeviceTelemetry(const char* destinationUrl, const char* hostname = nullptr);

#endif // TELEMETRY_CLIENT_H