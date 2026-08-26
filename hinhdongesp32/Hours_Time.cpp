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

void Hours_Time::time_server() {
    // Configura o serviço de tempo NTP (inicia o cliente em segundo plano)
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("\nServiço NTP configurado. Aguardando a primeira sincronização...");
}
void Hours_Time::calendar() {
    // Executa a checagem apenas a cada 10 segundos sem travar a CPU
    static unsigned long lastCalendarCheck = 0;
    if (millis() - lastCalendarCheck < 10000) return;
    lastCalendarCheck = millis();

    // 1. Só tenta buscar o tempo se a rede Wi-Fi estiver ativa
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NTP] Aguardando conexão Wi-Fi para sincronizar relógio...");
        return;
    }

    // 2. Tenta obter a data e hora do relógio interno
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100)) { // Timeout curto de 100ms em vez de travar
        Serial.println("Falha ao obter o tempo. Tentando novamente...");
        return;
    }
    
    // Imprime os detalhes no Monitor Serial
    Serial.println("--- Tempo Atual ---");
    
    char timeString[50];
    strftime(timeString, sizeof(timeString), "%d/%m/%Y %H:%M:%S", &timeinfo);
    Serial.printf("Data e Hora: %s\n", timeString);

    char dayOfWeek[10];
    strftime(dayOfWeek, sizeof(dayOfWeek), "%A", &timeinfo);
    Serial.printf("Dia da Semana: %s\n", dayOfWeek);

    Serial.println("-------------------");
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
    // 🧠 LÓGICA DE CÁLCULO DE PERÍODOS
    // -------------------------------------------------------------------------
    int atualMinutos = (timeinfo.tm_hour * 60) + timeinfo.tm_min;
    
    int sleepHour, sleepMin;
    sscanf(hours_sleep, "%d:%d", &sleepHour, &sleepMin);
    int sleepMinutos = (sleepHour * 60) + sleepMin;

    int wakeonHour, wakeonMin;
    sscanf(hours_wakeon, "%d:%d", &wakeonHour, &wakeonMin);
    int wakeonMinutos = (wakeonHour * 60) + wakeonMin;

    // 1. Período Noturno (Janela de sono completa, ex: entre 22:00 e 06:00)
    bool periodoSono = false;
    if (sleepMinutos > wakeonMinutos) {
        periodoSono = (atualMinutos >= sleepMinutos || atualMinutos < wakeonMinutos);
    } else {
        periodoSono = (atualMinutos >= sleepMinutos && atualMinutos < wakeonMinutos);
    }

    // 2. Janela Pré-Dormir (1 hora antes do horário de dormir)
    int janelaPreDormir = (sleepMinutos - 60 + 1440) % 1440;
    bool dentroDaJanela = false;
    if (janelaPreDormir < sleepMinutos) {
        dentroDaJanela = (atualMinutos >= janelaPreDormir && atualMinutos < sleepMinutos);
    } else {
        dentroDaJanela = (atualMinutos >= janelaPreDormir || atualMinutos < sleepMinutos);
    }

    // -------------------------------------------------------------------------
    // 🧠 DISPARO DA CATEGORIA (PRE-DORMIR OU MADRUGADA AO RELIGAR)
    // -------------------------------------------------------------------------
    static bool primeiraExecucao = true;

    // Se estiver no período de 1h antes OU já no horário de dormir (ao religar)
    if (dentroDaJanela || periodoSono) {
        if (!_categoriaAlterada || primeiraExecucao) {
            enviarAlteracaoCategoria("bedtime");
            _categoriaAlterada = true; 
            primeiraExecucao = false;
        }
    } 
    // Ao atingir o horário de acordar ou durante o dia, redefine para o padrão
    else {
        if (_categoriaAlterada || primeiraExecucao) {
            enviarAlteracaoCategoria("animation");
            _categoriaAlterada = false; // Reseta a flag para o próximo ciclo diário
            primeiraExecucao = false;
        }
    }

    // -------------------------------------------------------------------------
    // 🧠 CONTROLE DE TIMEOUT (MODO MANUAL)
    // -------------------------------------------------------------------------
    if (is_manual_mode) {
        if (millis() - manual_on_timestamp >= TIMEOUT_MS) {
            // AÇÃO: Timeout de 5 minutos atingido. Desliga o display
            if (animationRef) animationRef->control_oled_power(false);
            
            // 🎯 Dispara a categoria para o backend ao encerrar o modo manual
            if (periodoSono || dentroDaJanela) {
                enviarAlteracaoCategoria("bedtime");
            } else {
                enviarAlteracaoCategoria("animation");
            }

            is_manual_mode = false; // Sai do modo manual
            Serial.println("Timeout de 5 minutos atingido. Desligando display e atualizando categoria.");
        }
        return; 
    }

    // -------------------------------------------------------------------------
    // --- CONTROLE AUTOMÁTICO DE ENERGIA DO DISPLAY ---
    // -------------------------------------------------------------------------
    static int ultimoMinutoExecutado = -1;

    if (timeinfo.tm_min != ultimoMinutoExecutado) {
        if (strncmp(currentTimeStr, hours_sleep, 5) == 0) {
            if (animationRef && animationRef->is_oled_on()) {
                Serial.println("Hora de Dormir atingida!");
                animationRef->control_oled_power(false);
                ultimoMinutoExecutado = timeinfo.tm_min;
            }
        } 
        else if (strncmp(currentTimeStr, hours_wakeon, 5) == 0) {
            if (animationRef && !animationRef->is_oled_on()) {
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