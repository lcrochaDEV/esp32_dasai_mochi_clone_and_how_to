#include "WString.h"
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include "WirelessConnection.h"

#include "WifiConnect.h"

// Variável para contar as tentativas
  int maxTentativas = 10;
  int tentativaAtual = 0;

WifiConnect::WifiConnect(const char* ssid, const char* password, Animations* animationPtr)
  : ssid(ssid), password(password), wifiAnimationRef(animationPtr)
{}

void WifiConnect::connections_Wifi(){
  //Serial.begin(115200);
  Serial.printf("Conectando a %s ", ssid);
  WiFi.begin(ssid, password); // Inicia a conexão

  while (WiFi.status() != WL_CONNECTED) { // Aguarda a conexão ser estabelecida
    delay(500);
    Serial.print(".");
    if(tentativaAtual == maxTentativas){
      wifiAnimationRef->not_wifi();
      Serial.println("\nFalha ao conectar Wifi!");
      Serial.print("\nConectando");
    }
    tentativaAtual++; // Incrementa o contador
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nConectado ao Wi-Fi!");
    Serial.print("Endereco IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Endereco MAC do Gateway: ");
    Serial.println(WiFi.macAddress()); // Anote este MAC para usar no codigo do Sender
    Serial.print("Canal Wi-Fi atual: ");
    Serial.println(WiFi.channel()); // Todos os senders devem usar este canal
  }
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
  Serial.print(n);
  Serial.println(" redes encontradas");

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
        return (encryptionType == ENC_TYPE_NONE) ? "Aberta" : "Protegida (WPA/WPA2)";
    #endif
}