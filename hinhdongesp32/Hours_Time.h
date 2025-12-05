#ifndef HOURS_TIME_H
#define HOURS_TIME_H

#include <Arduino.h>

#include "Animations.h"

class Hours_Time {
  public:
    Hours_Time(const char* hours_up = "", const char* hours_down = "", const char* date = "", long gmtOffset_sec = -3 * 3600, int daylightOffset_sec = 0, const char* ntpServer = "pool.ntp.org",  Animations* animationPtr = nullptr);
    //METODO DE TODO O PROGRAMA
    void time_server();
    void weke_on();
  private:
    // Configurações de Fuso Horário e NTP
    // Fuso horário de Brasília (GMT -3)
    const char* hours_up;
    const char* hours_down;
    const char* date;
    long gmtOffset_sec;
    int daylightOffset_sec; // 0 para não usar Horário de Verão
    const char* ntpServer;
    void calendar();
    Animations* animationRef;
};
 
#endif