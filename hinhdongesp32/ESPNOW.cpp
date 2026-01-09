#include "EspNow.h"
#include <string.h>

struct_message EspNow::myData;
int EspNow::_staticLedPin = -1;
const char* EspNow::_tagNameStatic = nullptr;

//Callback quando os dados sao enviados
#if defined(ESP32)
void EspNow::OnDataSent(const esp_now_send_info_t *mac_addr, esp_now_send_status_t status) {
    Serial.printf("\nStatus do envio: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
}
#else
void EspNow::OnDataSent(uint8_t *mac_addr, uint8_t status) {
    Serial.printf("\nStatus do envio: %s\n", status == 0 ? "Sucesso" : "Falha");
}
#endif

#if defined(ESP32)
void EspNow::OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.print("Bytes recebidos: ");
    Serial.println(len);
    Serial.print("String: ");
    Serial.println(myData.a);
    
    processCommand(myData.a);
}
#else
void EspNow::OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
    memcpy(&myData, incomingData, sizeof(myData));
    processCommand(myData.a);
}
#endif

EspNow::EspNow(const std::array<uint8_t, 6>& broadcastAddress, int pinNumber, const char* tagName)
    : broadcastAddress(broadcastAddress), pinNumber(pinNumber), tagName(tagName)
{
    _staticLedPin = pinNumber; // Atribui o pino à variável estática para uso nos callbacks
    _tagNameStatic = tagName;      // Atribui à variável estática para uso no OnDataRecv
}
void EspNow::processCommand(const char* message) {
    char cmdOn[32], cmdOff[32];
    const char* tag = EspNow::_tagNameStatic ? EspNow::_tagNameStatic : "Dispositivo";
    snprintf(cmdOn, sizeof(cmdOn), "On_%s", tag);
    snprintf(cmdOff, sizeof(cmdOff), "Off_%s", tag);

    if (strcmp(message, cmdOn) == 0) digitalWrite(EspNow::_staticLedPin, HIGH);
    else if (strcmp(message, cmdOff) == 0) digitalWrite(EspNow::_staticLedPin, LOW);
}
void EspNow::nowSetup() {
  //Serial.begin(115200);

    //Inicializa o pino do botao
    if (pinNumber != -1) pinMode(pinNumber, INPUT);

    //Coloca o dispositivo no modo Wi-Fi Station
    WiFi.mode(WIFI_STA);

    //Inicializa o ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Erro ao inicializar o ESP-NOW");
        return;
    }
    #if defined(ESP8266)
        esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
        esp_now_register_send_cb(OnDataSent);
        esp_now_register_recv_cb(OnDataRecv);
        esp_now_add_peer(broadcastAddress.data(), ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    #elif defined(ESP32)
        esp_now_register_send_cb(OnDataSent);
        esp_now_register_recv_cb(OnDataRecv);
        
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, broadcastAddress.data(), 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Falha ao adicionar peer");
        }
    #endif
}
void EspNow::beginRunSent() {

    if (pinNumber != -1 && digitalRead(pinNumber) == HIGH) { // HIGH ou 1, dependendo da sua ligação
        while (digitalRead(pinNumber) == HIGH) {
        delay(50);
    }
    Serial.println("Botao pressionado!");
    //Inverte o estado do led
    inverte_led = !inverte_led;
    char cmdOn[32], cmdOff[32];
    const char* tag = EspNow::_tagNameStatic ? EspNow::_tagNameStatic : "Dispositivo";
    snprintf(cmdOn, sizeof(cmdOn), "On_%s", tag);
    snprintf(cmdOff, sizeof(cmdOff), "Off_%s", tag);
    //Envia a string de acordo com o estado do led
    if (inverte_led) strncpy(myData.a, cmdOn, sizeof(myData.a));
    else strncpy(myData.a, cmdOff, sizeof(myData.a));
    // 5. EXECUÇÃO LOCAL (Chama o processador de comandos)
    // Isso faz o LED do próprio dispositivo ligar/desligar
    EspNow::processCommand(myData.a);
    // Envia a mensagem usando o ESP-NOW
    esp_now_send(broadcastAddress.data(), (uint8_t *) &myData, sizeof(myData));
  }
}

void EspNow::updateSent(uint32_t intervalMs) {
    uint32_t tempoAtual = millis();

    // Técnica de millis() integrada à classe
    if (tempoAtual - _ultimoTempo >= intervalMs) {
        _ultimoTempo = tempoAtual;
        
        // Chama a função de leitura e envio
        beginRunSent(); 
    }
}

void EspNow::beginRunRecv() {
    if (_staticLedPin != -1) {
        pinMode(_staticLedPin, OUTPUT);
        digitalWrite(_staticLedPin, LOW);
    }
    Serial.println("Receptor pronto.");
}