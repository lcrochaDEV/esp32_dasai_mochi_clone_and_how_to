#ifndef PHYSICALACCESSCONTROL_H
#define PHYSICALACCESSCONTROL_H

#include <Arduino.h>

class PhysicalAccessControl {
  public:
    PhysicalAccessControl();
      String  modelBoardESP();
      String total_ran();
      String flash_size();
      String menor_ran_size();
      String sketch_Size();
      String pinGPIO();
};

#endif