#ifndef SDDATA_H
#define SDDATA_H

#include <Arduino.h>

#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

class SDData {
  public:
    SDData(const char* filename = nullptr);
    void sdbegin();
    bool createFolder(const char * path); // Novo método
    void listDir(const char * dirname, uint8_t levels);
  protected:
  const char* filename;

};

#endif                
