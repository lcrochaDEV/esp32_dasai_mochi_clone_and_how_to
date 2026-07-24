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


void Hours_Time::weke_on() {
    // Roda a checagem 1 vez por segundo para economizar CPU
    static unsigned long lastTimeCheck = 0;
    if (millis() - lastTimeCheck < 1000) return;
    lastTimeCheck = millis();

    // 1. Declara onde a hora será armazenada
    struct tm timeinfo; 
    if (!getLocalTime(&timeinfo)) return;

    // 2. Declara onde a string formatada será armazenada
    char currentTimeStr[6]; 
    // 3. Formata a hora para a string (ex: de números para "18:00")
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

    // Proteção profissional para virada de dia (Adiciona 1440 min se der negativo)
    int janelaPreDormir = (sleepMinutos - 60 + 1440) % 1440;

    // Verifica se a janela cruza a meia-noite (ex: se dorme às 00:30, janela é 23:30)
    bool dentroDaJanela = false;
    if (janelaPreDormir < sleepMinutos) dentroDaJanela = (atualMinutos >= janelaPreDormir && atualMinutos < sleepMinutos); // Caso padrão no mesmo dia (ex: janela 21:00 às 22:00)
    else dentroDaJanela = (atualMinutos >= janelaPreDormir || atualMinutos < sleepMinutos); // Caso cruze a meia-noite (ex: janela 23:30 às 00:30)

    static bool primeiraExecucao = true;

    if (dentroDaJanela) {
        if (!_categoriaAlterada || primeiraExecucao) {
            enviarAlteracaoCategoria("bedtime");
            _categoriaAlterada = true; 
            primeiraExecucao = false;
        }
    } 
    // Ao atingir o horário de acordar (Wakeon) ou durante o dia, redefine para o padrão
    else {
        if (_categoriaAlterada || primeiraExecucao) {
            enviarAlteracaoCategoria("animation");
            _categoriaAlterada = false; // Reseta a flag para o próximo ciclo diário
            primeiraExecucao = false;
        }
    }
    // -------------------------------------------------------------------------

    // --- PARTE NOVA: CONTROLE DE TIMEOUT ---
    // Verifica se estamos no modo manual e se o tempo expirou
    if (is_manual_mode) {
        // millis() retorna o tempo em ms desde o boot
        if (millis() - manual_on_timestamp >= TIMEOUT_MS) {
            // AÇÃO: Timeout de 5 minutos atingido. Desliga o display
            if (animationRef) animationRef->control_oled_power(false);
            is_manual_mode = false; // Sai do modo manual
            Serial.println("Timeout de 5 minutos atingido. Desligando display.");
        }
        // Se ainda estiver no modo manual e o tempo não expirou, a função termina aqui
        // para manter o display ligado até o timeout.
        return; 
    }

    // --- CONTROLE AUTOMÁTICO DE ENERGIA DO DISPLAY ---
    // Trava de borda: Garante que o comando de liga/desliga só execute 1 vez no minuto
    static int ultimoMinutoExecutado = -1;

    if (timeinfo.tm_min != ultimoMinutoExecutado) {
        // 4. Compara a string formatada com a sua string de referência
        if (strncmp(currentTimeStr, hours_sleep, 5) == 0) {
            if (animationRef && animationRef->is_oled_on()) {
                // 🎯 AÇÃO: Desliga o display
                Serial.println("Hora de Dormir atingida!");
                animationRef->control_oled_power(false);
                ultimoMinutoExecutado = timeinfo.tm_min;
            }
        } 
        else if (strncmp(currentTimeStr, hours_wakeon, 5) == 0) {
            if (animationRef && !animationRef->is_oled_on()) {
                // 🎯 AÇÃO: Liga o display
                Serial.println("Hora de Ligar atingida!");
                animationRef->control_oled_power(true);
                ultimoMinutoExecutado = timeinfo.tm_min;
            } 
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
        strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M:%S", &timeinfo); 
        // 2. Verifica se estamos no período ATIVO (22:00 até 06:00)
        if (strncmp(currentTimeStr, hours_sleep, 5) >= 0 || strncmp(currentTimeStr, hours_wakeon, 5) < 0) {    
            // AÇÃO: Liga o display
            if (animationRef) animationRef->control_oled_power(true);
            
            // Define o modo manual e salva o tempo atual (millis())
            is_manual_mode = true;
            manual_on_timestamp = millis();
            
            Serial.println("Display ligado manualmente (Modo Timeout).");
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

    http.begin(url);
    int httpResponseCode = http.POST(""); // Envia o POST vazio conforme a estrutura do curl

    if (httpResponseCode > 0) Serial.printf("[HTTP] Categoria alterada para '%s'. Resposta: %d\n", novaCategoria, httpResponseCode);
    else Serial.printf("[HTTP] Erro ao enviar POST: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
}