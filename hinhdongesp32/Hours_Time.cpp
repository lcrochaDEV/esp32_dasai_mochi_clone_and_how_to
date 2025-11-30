#include "time.h"

#include "Hours_Time.h"

Hours_Time::Hours_Time(long  gmtOffset_sec, int daylightOffset_sec, const char* ntpServer){
  // Configurações de Fuso Horário e NTP
  // Fuso horário de Brasília (GMT -3)
  this->gmtOffset_sec = gmtOffset_sec;
  this->daylightOffset_sec = daylightOffset_sec;  // 0 para não usar Horário de Verão
  this->ntpServer = ntpServer;
}

void Hours_Time::time_server(){
  // 2. Configuração do Serviço de Tempo (NTP)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
  Serial.println("Serviço NTP configurado. Aguardando a primeira sincronização...");
  calendar();
}

void Hours_Time::calendar() {
  // Estrutura para armazenar a informação de tempo
  struct tm timeinfo;

  // Tenta obter a data e hora do RTC interno (sincronizado pelo NTP)
  if(!getLocalTime(&timeinfo)){
    Serial.println("Falha ao obter o tempo. Tentando novamente...");
    delay(5000);
    return;
  }
  
  // Imprime os detalhes no Monitor Serial
  Serial.println("--- Tempo Atual ---");
  
  // Formata a data e hora (Ex: 29/11/2025 12:05:21)
  char timeString[50];
  strftime(timeString, sizeof(timeString), "%d/%m/%Y %H:%M:%S", &timeinfo);
  Serial.print("Data e Hora: ");
  Serial.println(timeString);

  // Exibe o dia da semana (Ex: Sexta-feira)
  char dayOfWeek[10];
  strftime(dayOfWeek, sizeof(dayOfWeek), "%A", &timeinfo);
  Serial.print("Dia da Semana: ");
  Serial.println(dayOfWeek);

  Serial.println("-------------------");

  // Espera 10 segundos antes de buscar novamente
  delay(10000); 
}

void Hours_Time::weke_on(){

}