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

void WifiConnect::searchRedes(){
  int n = WiFi.scanNetworks(); // Escaneia redes
  Serial.print(n);
  Serial.println(" redes encontradas");

  if (n == 0) {
    Serial.println("Nenhuma rede encontrada.");
  } else {
    for (int i = 0; i < n; ++i) {
      // Exibe SSID, RSSI e tipo de segurança
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i)); // Sinal mais próximo de 0 é mais forte
      Serial.print("dBm)");
      if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
        Serial.println(" - Aberta");
      } else {
        Serial.println(" - Protegida");
      }
    }
  }
}