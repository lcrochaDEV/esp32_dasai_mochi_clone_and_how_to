#ifndef HOURS_TIME_H
#define HOURS_TIME_H

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <Arduino.h>
#include <HTTPClient.h>
#include "Animations.h"

class Hours_Time {
private:
    // Mantidos exatamente como originalmente definidos
    const char* hours_sleep;
    const char* hours_wakeon;
    const char* date;
    long gmtOffset_sec;
    int daylightOffset_sec;
    const char* ntpServer;
    Animations* animationRef;

    // Atributos privados para controle interno de estado
    int _sleepMinutos;
    int _wakeonMinutos;
    bool _categoriaAlterada;
    bool is_manual_mode;
    unsigned long manual_on_timestamp;
    const unsigned long TIMEOUT_MS = 300000; // 5 minutos

    // Estrutura do Relógio Interno
    time_t _epochBase;
    unsigned long _millisBase;
    bool _isSynced;

    // Métodos utilitários internos
    int _parseTimeToMinutes(const char* timeStr) const;
    void _checkNTPSync();
    time_t _getInternalEpoch() const;

public:
    Hours_Time(const char* hours_sleep, const char* hours_wakeon, const char* date, 
               long gmtOffset_sec, int daylightOffset_sec, const char* ntpServer, 
               Animations* animationPtr);

    const char* getHoursWakeon() const;
    const char* getHoursSleep() const;

    void time_server();
    void calendar();
    void weke_on();
    void manual_turn_on();
    const char* losttime() const;

    void enviarAlteracaoCategoria(const char* novaCategoria);
    bool _enviarComandoDelay(float segundos = 0.09);
};

#endif // HOURS_TIME_H