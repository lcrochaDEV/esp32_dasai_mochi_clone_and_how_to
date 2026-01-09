//#include <BLEDevice.h>
//#include <BLEScan.h>
//#include "BluetoothSerial.h"

//BluetoothSerial SerialBT;

#include "BTConnect.h"

BTConnect::BTConnect() {
    // Construtor
}

void BTConnect::btbegin() {

}
/*
// --- SCAN CLASSIC ---
void BTConnect::btClassicScan() {
  Serial.println("\n--- INICIANDO SCAN CLASSIC (BR/EDR) ---");
  
  // O Classic precisa estar ativo para escanear
  if (!SerialBT.begin("ESP32_Scanner")) {
    Serial.println("Erro ao iniciar BT Classic");
    return;
  }

  // discover(ms) retorna uma lista de dispositivos encontrados
  // Nota: Este processo é bloqueante e dura o tempo definido (ex: 10s)
  BTScanResults* pResults = SerialBT.discover(15000); 
  
  if (pResults && pResults->getCount() > 0) {
    Serial.printf("Encontrados %d dispositivos Classic:\n", pResults->getCount());

    for (int i = 0; i < pResults->getCount(); i++) {
      BTAdvertisedDevice* dev = pResults->getDevice(i);

      // Obtém o Class of Device (COD)
      uint32_t cod = dev->getCOD();
      
      // Extração de informações de Segurança/Criptografia do COD
      // O bit 13 do COD indica se o dispositivo suporta criptografia a nível de informação
      bool encryptionCap = (cod & (1 << 13)); 

    Serial.printf("  [Classic] MAC: %s | Nome: %-20s | RSSI: %d | Encrypto: %s\n", 
                    dev->getAddress().toString().c_str(),
                    dev->getName().length() > 0 ? dev->getName().c_str() : "<Desconhecido>",
                    dev->getRSSI(),
                    encryptionCap ? "Suportada (AES/E0)" : "Nao Informada");
    }
  } else {
    Serial.println("Nenhum dispositivo Classic encontrado.");
  }

  SerialBT.end(); // Fecha o Classic para liberar o rádio para o BLE
}

// --- SCAN BLE ---
void BTConnect::btBLEScan() {
  Serial.println("\n--- INICIANDO SCAN BLE (Low Energy) ---");
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  
  BLEScanResults* foundDevices = pBLEScan->start(5, false);
  
  for (int i = 0; i < foundDevices->getCount(); i++) {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    
    // No BLE, se o dispositivo é conectável, ele obrigatoriamente suporta AES-128
    // Verificamos via flags de advertising
    String bleCrypto = "Aberto/Beacon";
    if (device.haveAppearance() || device.getPayloadLength() > 0) {
        bleCrypto = "AES-128 CCM";
    }

    Serial.printf("  [BLE]     MAC: %s | Nome: %-15s | RSSI: %d | Encrypto: %s\n", 
                  device.getAddress().toString().c_str(), 
                  device.haveName() ? device.getName().c_str() : "<Desconhecido>",
                  device.getRSSI(),
                  bleCrypto.c_str());
  }
  pBLEScan->clearResults();
  BLEDevice::deinit(false);
}
*/