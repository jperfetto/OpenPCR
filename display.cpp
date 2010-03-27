#include "pcr_includes.h"
#include "display.h"

#include "thermocycler.h"
#include "program.h"

Display::Display(Thermocycler& thermocycler):
  iLcd(7, 8, 9, 10, 11, 12),
  iThermocycler(thermocycler),
  ipDisplayCycle(NULL) {

  iLcd.begin(20, 4);
}

void Display::Update() {
  //heat/cool status
  iLcd.setCursor(0, 0);
  iLcd.print("Heating");
  
  char floatStr[10];
  sprintFloat(floatStr, iThermocycler.GetPlateTemp(), 1);
  char buf[32];
  sprintf(buf, "%s C", floatStr);
 
  iLcd.setCursor(13, 0);
  iLcd.print(buf);
 
  //Stage
  iLcd.setCursor(0, 1);
  iLcd.print(ThermalStateString(iThermocycler.GetThermalState()));
 
  //Stage Details
  iLcd.setCursor(0, 2);
  iLcd.print("1 min @ 61C");
 
  //Cycle
  if (ipDisplayCycle != NULL) {
    iLcd.setCursor(0, 3);
    sprintf(buf, "(%d of %d)", ipDisplayCycle->GetCurrentCycle(), ipDisplayCycle->GetNumCycles());
    iLcd.print(buf);
  }
 
  //Time Remaining
  iLcd.setCursor(11, 3);
  iLcd.print("ETA: 1:32");
}

char* Display::ThermalStateString(Thermocycler::ThermalState state) {
  switch (state) {
  case Thermocycler::EHolding:
    return "Holding";
  case Thermocycler::EHeating:
    return "Heating";
  case Thermocycler::ECooling:
    return "Cooling";
  default:
    return "Error";
  }
}
