#include "time.h"

#include "Hours_Time.h"

Hours_Time::Hours_Time(const char* hours_up, const char* hours_down, const char* date, long  gmtOffset_sec, int daylightOffset_sec, const char* ntpServer, Animations* animationPtr){
  // Configurações de Fuso Horário e NTP
  // Fuso horário de Brasília (GMT -3)
  this->hours_up = hours_up;
  this->hours_down = hours_down;
  this->date = date;
  this->gmtOffset_sec = gmtOffset_sec;
  this->daylightOffset_sec = daylightOffset_sec;  // 0 para não usar Horário de Verão
  this->ntpServer = ntpServer;
  // Salva a referência do objeto Animations
  this->animationRef = animationPtr;
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
    struct tm timeinfo; // 1. Declara onde a hora será armazenada
    if (getLocalTime(&timeinfo)) { // 2. Pega a hora (verifica se foi sucesso)
        char currentTimeStr[6]; // 3. Declara onde a string formatada será armazenada
        // 4. Formata a hora para a string (ex: de números para "18:00")
        strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M", &timeinfo); 
        // 5. Compara a string formatada com a sua string de referência
        if (strcmp(this->hours_down, currentTimeStr) == 0) {
            //Serial.println("Hora de Desligamento atingida!");
            // 🎯 AÇÃO: Desliga o display
            this->animationRef->control_oled_power(false);
        }else if (strcmp(this->hours_up, currentTimeStr) == 0){
            Serial.println("Hora de Ligar atingida!");
            // 🎯 AÇÃO: Liga o display
            this->animationRef->control_oled_power(true);
        }
    }
}