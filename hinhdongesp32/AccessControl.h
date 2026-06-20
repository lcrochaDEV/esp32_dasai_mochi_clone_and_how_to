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