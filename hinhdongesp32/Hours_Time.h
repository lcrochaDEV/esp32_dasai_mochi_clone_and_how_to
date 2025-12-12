#ifndef HOURS_TIME_H
#define HOURS_TIME_H

#include <Arduino.h>

#include "Animations.h"

class Hours_Time {
  public:
    Hours_Time(const char* hours_sleep = "--:--", const char* hours_wakeon = "--:--", const char* date = "", long gmtOffset_sec = -3 * 3600, int daylightOffset_sec = 0, const char* ntpServer = "pool.ntp.org", Animations* animationPtr = nullptr);
    //METODO DE TODO O PROGRAMA
    void time_server();
    void weke_on();
    void manual_turn_on();
    const char* getHoursSleep() const {
        return hours_sleep;
    }
    const char* getHoursWakeon() const {
        return hours_wakeon;
    }
  private:
    // Configurações de Fuso Horário e NTP
    // Fuso horário de Brasília (GMT -3)
    const char* hours_sleep;   // hours_sleep: Deve ser o início do período noturno (22:00).
    const char* hours_wakeon;  // hours_wakeon: Deve ser o fim do período noturno (06:00).
    const char* date;
    long gmtOffset_sec;
    int daylightOffset_sec; // 0 para não usar Horário de Verão
    const char* ntpServer;
    void calendar();
    //CLASS ANIMATIONS
    Animations* animationRef;
    // Novo: Flag que indica se o display está em modo de timeout (ligado manualmente)
    bool is_manual_mode = false; 

    // Novo: Armazena o tempo (em milissegundos) em que o display foi ligado manualmente
    unsigned long manual_on_timestamp = 0; 

    // Novo: Constante para o tempo limite (5 minutos)
    const unsigned long TIMEOUT_MS = 1 * 60 * 1000; 
};
 
#endif