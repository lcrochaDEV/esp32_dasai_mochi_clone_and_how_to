#ifndef ACCESSCONTROL_H
#define ACCESSCONTROL_H

#include <Arduino.h>

#include "PhysicalAccessControl.h"
#include "FileSystemControl.h"

class AccessControl: public PhysicalAccessControl, public FileSystemControl {
  public:
    AccessControl (const char* ssid = nullptr, const char* password = nullptr);
};
 
#endif


//MUDAR ANIMAÇÃO POR SHELL
// curl -X POST "http://192.168.1.252:8003/set-delay?seconds=0.09"
