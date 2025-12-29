#ifndef SDDATA_H
#define SDDATA_H

#include <Arduino.h>

#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

class SDData {
  public:
    SDData(const char* filename = nullptr);
    void sdbegin();
    bool createFolder(const char * filename); // Novo método
    void listDir(const char * dirname, uint8_t levels);
    void createJSON(JsonDocument& doc);
    bool readJSON(JsonDocument& doc);
    void updateJSON(const char* chave, const char* novoValor);
    void deleteArquivo();
  protected:
  const char* filename;

};

#endif                
