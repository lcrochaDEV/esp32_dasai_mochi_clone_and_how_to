#include "Hours_Time.h"

Hours_Time::Hours_Time(const char* hours_sleep, const char* hours_wakeon, const char* date, long  gmtOffset_sec, int daylightOffset_sec, const char* ntpServer, Animations* animationPtr)
    : hours_sleep(hours_sleep), hours_wakeon(hours_wakeon), date(date), gmtOffset_sec(gmtOffset_sec), daylightOffset_sec(daylightOffset_sec), ntpServer(ntpServer), animationRef(animationPtr) {
}

const char* Hours_Time::getHoursWakeon() const {
    return hours_wakeon;
}

const char* Hours_Time::getHoursSleep() const {
    return hours_sleep;
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

/*
void Hours_Time::weke_on(){
    // --- PARTE NOVA: CONTROLE DE TIMEOUT ---
    // Verifica se estamos no modo manual E se o tempo expirou
    if (is_manual_mode) {
        // millis() retorna o tempo em ms desde o boot
        if (millis() - manual_on_timestamp >= TIMEOUT_MS) {
            // AÇÃO: Timeout de 5 minutos atingido. Desliga o display
            animationRef->control_oled_power(false);
            is_manual_mode = false; // Sai do modo manual
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
        if (strcmp(hours_sleep, currentTimeStr) == 0) {
            //Serial.println("Hora de Desligamento atingida!");
            // 🎯 AÇÃO: Desliga o display
            animationRef->control_oled_power(false);
        }else if (strcmp(hours_wakeon, currentTimeStr) == 0){
            Serial.println("Hora de Ligar atingida!");
            // 🎯 AÇÃO: Liga o display
            animationRef->control_oled_power(true);
        } 
    }
}
*/
// Esta função é chamada por um evento externo (p. ex., um botão)
void Hours_Time::manual_turn_on() {
    // 1. Pega a hora atual (necessário para checar se estamos no período 22:00-06:00)
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
       char currentTimeStr[6]; // 3. Declara onde a string formatada será armazenada
        // 4. Formata a hora para a string (ex: de números para "18:00")
        strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M:%S", &timeinfo); 
        // 2. Verifica se estamos no período ATIVO (22:00 até 06:00)
        if (strncmp(currentTimeStr, hours_sleep, 5) >= 0 || strncmp(currentTimeStr, hours_wakeon, 5) < 0) {    
            // AÇÃO: Liga o display
            animationRef->control_oled_power(true);
            
            // Define o modo manual e salva o tempo atual (millis())
            is_manual_mode = true;
            manual_on_timestamp = millis();
            
            Serial.println("Display ligado manualmente (Modo Timeout).");
        }
    }
}

void Hours_Time::weke_on() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    char currentTimeStr[6]; 
    strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M", &timeinfo);

    // -------------------------------------------------------------------------
    // 🧠 LÓGICA INTELIGENTE: Controle de Categoria Pré-Desligamento (1h antes)
    // -------------------------------------------------------------------------
    // Convertemos tudo para minutos desde o início do dia para facilitar a matemática limpa
    int atualMinutos = (timeinfo.tm_hour * 60) + timeinfo.tm_min;
    
    int sleepHour, sleepMin;
    sscanf(hours_sleep, "%d:%d", &sleepHour, &sleepMin);
    int sleepMinutos = (sleepHour * 60) + sleepMin;

    int wakeonHour, wakeonMin;
    sscanf(hours_wakeon, "%d:%d", &wakeonHour, &wakeonMin);
    int wakeonMinutos = (wakeonHour * 60) + wakeonMin;

    // Calcula a janela de 1 hora antes de dormir (60 minutos)
    int janelaPreDormir = sleepMinutos - 60;

    if (atualMinutos >= janelaPreDormir && atualMinutos < sleepMinutos) {
        if (!_categoriaAlterada) {
            enviarAlteracaoCategoria("hora_de_dormir");
            _categoriaAlterada = true; 
        }
    } 
    // Ao atingir o horário de acordar (Wakeon) ou durante o dia, redefine para o padrão
    else if (atualMinutos >= wakeonMinutos && atualMinutos < janelaPreDormir) {
        if (_categoriaAlterada) {
            enviarAlteracaoCategoria("animation");
            _categoriaAlterada = false; // Reseta a flag para o próximo ciclo diário
        }
    }
    // -------------------------------------------------------------------------

    // --- MODO MANUAL COM TIMEOUT ---
    if (is_manual_mode) {
        if (millis() - manual_on_timestamp >= TIMEOUT_MS) {
            is_manual_mode = false; 
            Serial.println("Timeout manual atingido. Retornando ao controle automatico.");
        } else {
            return; 
        }
    }

    // --- CONTROLE AUTOMÁTICO DE ENERGIA DO DISPLAY ---
    if (strncmp(currentTimeStr, hours_sleep, 5) == 0) {
        if (animationRef && animationRef->is_oled_on()) {
            // 🎯 AÇÃO: Desliga o display
            Serial.println("Hora de Dormir atingida!");
            animationRef->control_oled_power(false);
        }
    } 
    else if (strncmp(currentTimeStr, hours_wakeon, 5) == 0) {
        if (animationRef && !animationRef->is_oled_on()) {
            // 🎯 AÇÃO: Liga o display
            Serial.println("Hora de Ligar atingida!");
            animationRef->control_oled_power(true);
        } 
    }
}

const char* Hours_Time::losttime() const {
    struct tm timeinfo;
    static char timeString[50];
    if (getLocalTime(&timeinfo)) {
        strftime(timeString, sizeof(timeString), "%d/%m/%Y %H:%M:%S", &timeinfo);
        return timeString;
    }
    return "00/00/0000 00:00:00"; // Retorno de segurança
}

// Implemente o método de envio no final do seu Hours_Time.cpp
void Hours_Time::enviarAlteracaoCategoria(const char* novaCategoria) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] Erro: Sem conexão Wi-Fi para alterar categoria.");
        return;
    }

    HTTPClient http;
    // Constrói a URL exata solicitada apontando para o seu Worker Python
    String url = "http://192.168.1.252:8003/set-category?category=" + String(novaCategoria); 
    // OBS: Substitua '192.168.1.X' pelo IP real da máquina do seu Worker se não for localhost absoluto.

    http.begin(url);
    int httpResponseCode = http.POST(""); // Envia o POST vazio conforme a estrutura do curl

    if (httpResponseCode > 0) Serial.printf("[HTTP] Categoria alterada para '%s'. Resposta: %d\n", novaCategoria, httpResponseCode);
    else Serial.printf("[HTTP] Erro ao enviar POST: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
}