#ifndef WIFICONNECT_H
#define WIFICONNECT_H


#include <Arduino.h>

#include <WiFi.h>
#include "Animations.h"

class WifiConnect {
  public:
    WifiConnect(const char* ssid = nullptr, const char* password = nullptr, Animations* animationPtr = nullptr);
    void connections_Wifi();
    void backupRede();
    bool connections_status();
    void diconnectRede();
    void wifiOff();
    void searchRedes();
    String getEncryptionName(uint8_t encryptionType);
   
  private:
    String backupSsid;
    String backupPass;
    int maxTentativas = 20;
    int tentativaAtual = 0;
    bool ProcessoDeBackup = false;
  protected:
    const char* ssid; 
    const char* password;
    //CLASS ANIMATIONS
    Animations* wifiAnimationRef;
};
 
#endif