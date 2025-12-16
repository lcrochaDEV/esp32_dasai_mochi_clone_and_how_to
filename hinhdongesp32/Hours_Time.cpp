#include "time.h"

#include "Hours_Time.h"

Hours_Time::Hours_Time(const char* hours_sleep, const char* hours_wakeon, const char* date, long  gmtOffset_sec, int daylightOffset_sec, const char* ntpServer, Animations* animationPtr){
  // Configurações de Fuso Horário e NTP
  // Fuso horário de Brasília (GMT -3)
  this->hours_sleep = hours_sleep;
  this->hours_wakeon = hours_wakeon;
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
    
  Serial.println("\nServiço NTP configurado. Aguardando a primeira sincronização...");
  calendar();
  manual_turn_on();
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
    // --- PARTE NOVA: CONTROLE DE TIMEOUT ---
    // Verifica se estamos no modo manual E se o tempo expirou
    if (this->is_manual_mode) {
        // millis() retorna o tempo em ms desde o boot
        if (millis() - this->manual_on_timestamp >= this->TIMEOUT_MS) {
            // AÇÃO: Timeout de 5 minutos atingido. Desliga o display
            this->animationRef->control_oled_power(false);
            this->is_manual_mode = false; // Sai do modo manual
            Serial.println("Timeout de 5 minutos atingido. Desligando display.");
        }
        // Se ainda estiver no modo manual e o tempo não expirou, a função termina aqui
        // para manter o display ligado até o timeout.
        return; 
    }
    struct tm timeinfo; // 1. Declara onde a hora será armazenada
    if (getLocalTime(&timeinfo)) { // 2. Pega a hora (verifica se foi sucesso)
        char currentTimeStr[6]; // 3. Declara onde a string formatada será armazenada
        // 4. Formata a hora para a string (ex: de números para "18:00")
        strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M", &timeinfo);
        // 5. Compara a string formatada com a sua string de referência
        if (strcmp(this->hours_sleep, currentTimeStr) == 0) {
            //Serial.println("Hora de Desligamento atingida!");
            // 🎯 AÇÃO: Desliga o display
            this->animationRef->control_oled_power(false);
        }else if (strcmp(this->hours_wakeon, currentTimeStr) == 0){
            Serial.println("Hora de Ligar atingida!");
            // 🎯 AÇÃO: Liga o display
            this->animationRef->control_oled_power(true);
        } 
    }
}

// Esta função é chamada por um evento externo (p. ex., um botão)
void Hours_Time::manual_turn_on() {
    // 1. Pega a hora atual (necessário para checar se estamos no período 22:00-06:00)
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
       char currentTimeStr[6]; // 3. Declara onde a string formatada será armazenada
        // 4. Formata a hora para a string (ex: de números para "18:00")
        strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M", &timeinfo); 
        // 2. Verifica se estamos no período ATIVO (22:00 até 06:00)
        if (strcmp(currentTimeStr, this->hours_sleep) >= 0 || strcmp(currentTimeStr, this->hours_wakeon) < 0) {
            // AÇÃO: Liga o display
            this->animationRef->control_oled_power(true);
            
            // Define o modo manual e salva o tempo atual (millis())
            this->is_manual_mode = true;
            this->manual_on_timestamp = millis();
            
            Serial.println("Display ligado manualmente (Modo Timeout).");
        }
    }
}