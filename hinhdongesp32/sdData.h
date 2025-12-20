#ifndef SDDATA_H
#define SDDATA_H

#include "SD.h"
#include "SPI.h"

void sdRun() {
  if(!SD.begin()){
    Serial.println("Falha ao montar o Cartão SD!");
    return;
  }
  Serial.println("Cartao SD inicializado com sucesso!");
    // Opcional: imprime informacoes basicas sobre o cartao
    uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
      Serial.println("Nenhum cartao SD encontrado ou tipo desconhecido.");
    } else {
      Serial.print("Tipo de cartao SD: ");
      if (cardType == CARD_MMC) Serial.println("MMC");
      else if (cardType == CARD_SD) Serial.println("SDSC");
      else if (cardType == CARD_SDHC) Serial.println("SDHC");
      else Serial.println("DESCONHECIDO");

      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.printf("Tamanho do cartao: %lluMB\n", cardSize);
    }
}

#endif                
