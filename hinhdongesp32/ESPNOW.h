#ifndef ESPNOW_H
#define ESPNOW_H

#include <Arduino.h>

#include <array> 

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <espnow.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <esp_now.h>
#endif

typedef struct struct_message {
    char a[32]; // Usar array de char é mais seguro que String para ESP-NOW
} struct_message;


class EspNow {
  public:
    EspNow(const std::array<uint8_t, 6>& broadcastAddress, int pinNumber = -1, const char* tagName = nullptr);
    void nowSetup();
    void beginRunSent();
    void updateSent(uint32_t intervalMs = 50); //chamada no loop()
    void beginRunRecv();

  private:
    static void processCommand(const char* message);
    bool inverte_led = false;
    int valor;
    std::array<uint8_t, 6> broadcastAddress;     // Armazena o endereço MAC (6 bytes)
    int pinNumber;                    
    const char* tagName;
     
    // Armazena o ponteiro para o pino/senha
    static const char* _tagNameStatic;          // Tag Name dos Pinos/Apelidos
    static struct_message myData;                // Cria uma struct_message chamada myData
    uint32_t _ultimoTempo = 0;                   // Variáveis para o controle de tempo (millis)
    static int _staticLedPin;

    // O callback deve ser estático ou uma função global
    #if defined(ESP32)
        static void OnDataSent(const esp_now_send_info_t *mac_addr, esp_now_send_status_t status);
        static void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len);
    #elif defined(ESP8266)
        static void OnDataSent(uint8_t *mac_addr, uint8_t status);
        static void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len);
    #endif
};

#endif
//Programa: ESP-NOW com ESP8266 NodeMCU - Emissor
//Autor: Arduino e Cia