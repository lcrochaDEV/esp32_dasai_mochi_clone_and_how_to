#include "WString.h"
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include "WirelessConnection.h"
#include "WifiConnect.h"
#include "Hours_Time.h"


WifiConnect::WifiConnect(const char* ssid, const char* password, Animations* animationPtr)
  : ssid(ssid), password(password), wifiAnimationRef(animationPtr) {
    // Variável para contar as tentativas
    int maxTentativas;
    int tentativaAtual;
    bool ProcessoDeBackup;
  }

void WifiConnect::connections_Wifi(){
  // 1. Força a desconexão total para limpar o rádio
  WiFi.disconnect(true);
  delay(100);
  //Serial.begin(115200);
  Serial.printf("Conectando a %s ", ssid);
  // Configura para reconectar automaticamente se cair
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password); // Inicia a conexão

  // Reinicia contador para nova tentativa
  tentativaAtual = 0;

  while (WiFi.status() != WL_CONNECTED) {// Aguarda a conexão ser estabelecida
    delay(500);
    if (WiFi.status() == WL_CONNECTED) break;
    Serial.print(".");

    if(tentativaAtual == maxTentativas){
      wifiAnimationRef->not_wifi();
      Serial.println("\nFalha ao conectar Wifi!");
      return;
    }
    tentativaAtual++; // Incrementa o contador
  }
}

void WifiConnect::backupRede() {
  // 1. Registra os eventos APENAS UMA VEZ (geralmente no setup ou init da classe)
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    // Reinicia contador para nova tentativa
    if (ProcessoDeBackup) {
        Serial.println("\n[OK] Conectado via Backup!");
        ProcessoDeBackup = false; 
    } else {
        Serial.println("\n\n[OK] Conectado à rede principal!");
        Serial.printf("SSID: %s", WiFi.SSID().c_str());
        Serial.printf("Endereco IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Endereco MAC do Gateway %s\n", WiFi.macAddress().c_str()); // Anote este MAC para usar no codigo do Sender
        Serial.printf("Canal Wi-Fi atual: %d\n\n", WiFi.channel());
        backupSsid = WiFi.SSID();
        backupPass = WiFi.psk();
    }
  }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    uint8_t motivo = info.wifi_sta_disconnected.reason;
    if (ProcessoDeBackup) return; // Evita loop infinito se o backup também falhar
      // Motivos comuns de falha (Senha errada, AP não encontrado, etc)
      if (motivo == 2 || motivo == 202 || motivo == 201) {
          Serial.printf("\n[Falha] Motivo %d. Tentando backup...", motivo);
          ProcessoDeBackup = true;
          WiFi.disconnect(true, false); // Aborta a tentativa atual oficialmente
          WiFi.setAutoReconnect(false); // 2. Opcional: Desativa o auto-reconnect temporariamente para não conflitar
          // Usamos o begin para a rede de backup salva
          WiFi.begin(backupSsid.c_str(), backupPass.c_str()); // 4. Reativa se desejar que o backup também tente se manter
          WiFi.setAutoReconnect(true);
        return;
      }
  }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void WifiConnect::Uptime(){
  // 1. Registra os eventos APENAS UMA VEZ (geralmente no setup ou init da classe)
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    uint8_t motivo = info.wifi_sta_disconnected.reason;
    Serial.printf("✅ CONEXÃO RESTABELECIDA: %s\n", losttime());
  }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    static bool executado = false;
    if (executado) return; // Se já rodou, sai da função imediatamente
    Serial.printf("⚠️ QUEDA DE CONEXÃO: %s\n", losttime());
    executado = true; 
  }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

bool WifiConnect::connections_status(){
    return (WiFi.status() == WL_CONNECTED);
}

void WifiConnect::diconnectRede(){
    WiFi.disconnect(true); // Desconecta da rede
    Serial.println("Desligando WiFi...");
}
void WifiConnect::wifiOff(){
    diconnectRede();
    WiFi.mode(WIFI_OFF); // Desliga a interface Wi-Fi
    Serial.println("WiFi desligado.");
}

void WifiConnect::searchRedes() {
  int n = WiFi.scanNetworks(); // Escaneia redes
  Serial.printf("%d redes encontradas\n", n);

  if (n == 0) {
    Serial.println("Nenhuma rede encontrada.");
  } else {
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s (%d dBm) - Segurança: ", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));

      // Pegamos o tipo de criptografia
      uint8_t encryptionType = WiFi.encryptionType(i);
      Serial.println(getEncryptionName(encryptionType));
    }
  }
  // Limpa o resultado do scan da memória para evitar vazamentos
  WiFi.scanDelete();
}
// Função auxiliar para traduzir o tipo de criptografia
String WifiConnect::getEncryptionName(uint8_t encryptionType) {
    #if defined(ESP32)
        switch (encryptionType) {
            case WIFI_AUTH_OPEN:            return "Aberta";
            case WIFI_AUTH_WEP:             return "WEP";
            case WIFI_AUTH_WPA_PSK:         return "WPA-PSK";
            case WIFI_AUTH_WPA2_PSK:        return "WPA2-PSK";
            case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2-PSK";
            case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
            case WIFI_AUTH_WPA3_PSK:        return "WPA3-SAE";
            case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/WPA3 Transition";
            default:                        return "Protegida";
        }
    #elif defined(ESP8266)
      switch (encryptionType) {
          case ENC_TYPE_NONE: return "Aberta";
          case ENC_TYPE_WEP:  return "WEP";
          case ENC_TYPE_TKIP: return "WPA (TKIP)";
          case ENC_TYPE_CCMP: return "WPA2 (AES)";
          case ENC_TYPE_AUTO: return "WPA/WPA2 Auto";
          default:            return "Protegida";
      }
    #endif
}
