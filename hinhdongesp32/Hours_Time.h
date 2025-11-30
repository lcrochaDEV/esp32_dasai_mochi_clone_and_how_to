#ifndef HOURS_TIME_H
#define HOURS_TIME_H

#include <Arduino.h>

class Hours_Time {
  public:
    Hours_Time(long gmtOffset_sec = -3 * 3600, int daylightOffset_sec = 0, const char* ntpServer = "pool.ntp.org");
    //METODO DE TODO O PROGRAMA
    void time_server();
    void weke_on();
  private:
    // Configurações de Fuso Horário e NTP
    // Fuso horário de Brasília (GMT -3)
    long gmtOffset_sec;
    int daylightOffset_sec; // 0 para não usar Horário de Verão
    const char* ntpServer;
    void calendar();
};
 
#endif