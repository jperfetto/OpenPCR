#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <LiquidCrystal.h>
#include "thermocycler.h"

class Cycle;

class Display {
public:
  Display(Thermocycler& thermocycler);
  
  void SetDisplayCycle(Cycle* pDisplayCycle) { ipDisplayCycle = pDisplayCycle; }
  void Update();
  
private:
  char* ThermalStateString(Thermocycler::ThermalState state);
  
private:
  LiquidCrystal iLcd;
  Thermocycler& iThermocycler;
  Cycle* ipDisplayCycle;
  Thermocycler::ProgramState iLastState;
};

#endif

