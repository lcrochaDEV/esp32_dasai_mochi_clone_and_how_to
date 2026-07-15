#include "WifiConnect.h"

WifiConnect::WifiConnect(const char* ssid, const char* password, Animations* animationPtr)
  : ssid(ssid), password(password), wifiAnimationRef(animationPtr) {
  }

void WifiConnect::connections_Wifi() {

   // -------------------------------------------------------------------------
  // PASSO 1: Reset e Estabilização do rádio
  // -------------------------------------------------------------------------
  // Desconecta de redes anteriores e limpa as configurações salvas na memória flash
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(300); // Aguarda 300ms. Essencial para o chip C3 estabilizar o rádio físico

  Serial.printf("Conectando a %s ", ssid);

  // -------------------------------------------------------------------------
  // PASSO 2: Configuração do Modo Estação (STA)
  // -------------------------------------------------------------------------
  WiFi.mode(WIFI_STA);         // Configura o ESP32 como cliente Wi-Fi (Estação)
  WiFi.persistent(false);      // Evita o desgaste da memória Flash gravando dados toda hora
  WiFi.setAutoReconnect(true); // Diz ao Core do ESP32 para reconectar sozinho caso o sinal caia
  WiFi.begin(ssid, password);  // Dispara a tentativa de conexão em segundo plano

  // Reinicia o contador da classe para esta nova tentativa
  tentativaAtual = 0;

  // -------------------------------------------------------------------------
  // PASSO 3: Loop de Aguardo (Polling com limite de tentativas)
  // -------------------------------------------------------------------------
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Aguarda meio segundo entre as verificações
    
    // Alimenta o Watchdog do ESP32-C3. Dá tempo para o processador processar 
    // os pacotes de segurança (WPA2/WPA3) no background sem travar o chip.
    yield();
    
    Serial.print(".");

    tentativaAtual++; // Incrementa o contador
    
    if (tentativaAtual >= maxTentativas) { 
      // Se excedeu o limite (ex: 20 tentativas de 500ms = 10 segundos), cancela
      // Se quiser avisar o Mochi que está sem internet para ele mudar a animação da tela:
      if (wifiAnimationRef) wifiAnimationRef->not_wifi(); 
      Serial.println("\nFalha ao conectar Wifi!");
      return; // Sai da função imediatamente
    }

  // -------------------------------------------------------------------------
  // PASSO 4: Sucesso na Conexão & Backup
  // -------------------------------------------------------------------------
  // Se o fluxo saiu do 'while', significa que WiFi.status() virou WL_CONNECTED
    if (WiFi.status() == WL_CONNECTED) {
      // Guarda em RAM nas variáveis da classe
      backupSsid = WiFi.SSID();
      backupPass = WiFi.psk();
      break;
    }
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
        Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("Endereco IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Endereco MAC do Gateway %s\n", WiFi.macAddress().c_str()); // Anote este MAC para usar no codigo do Sender
        Serial.printf("Canal Wi-Fi atual: %d\n\n", WiFi.channel());
        backupSsid = WiFi.SSID();
        backupPass = WiFi.psk();
    }
  }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    uint8_t motivo = info.wifi_sta_disconnected.reason;
    if (!ProcessoDeBackup && backupSsid.length() > 0) {
      if (motivo == 2 || motivo == 202 || motivo == 201) {
          Serial.printf("\n[Falha] Motivo %d. Tentando backup...", motivo);
          ProcessoDeBackup = true;
          WiFi.disconnect(true, false); 
          WiFi.setAutoReconnect(false); 
          WiFi.begin(backupSsid.c_str(), backupPass.c_str()); 
          WiFi.setAutoReconnect(true);
          return;
      }
    } else if (backupSsid.length() == 0) {
        // Se a rede principal caiu na primeira tentativa, avisa o motivo real
        Serial.printf("\n[Falha de Autenticacao] Motivo %d. Verifique a senha!", motivo);
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
  Serial.printf("\n%d redes encontradas\n", n);

  if (n == 0) {
    Serial.println("\nNenhuma rede encontrada.");
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
