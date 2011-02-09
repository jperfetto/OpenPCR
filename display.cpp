#include "pcr_includes.h"
#include "display.h"

#include "thermocycler.h"
#include "program.h"

Display::Display(Thermocycler& thermocycler):
  iLcd(6, 7, 8, A5, 16, 17),
  iThermocycler(thermocycler),
  ipDisplayCycle(NULL),
  iLastState(Thermocycler::EOff) {

  iLcd.begin(20, 4);
}

void Display::Update() {
  Thermocycler::ProgramState state = iThermocycler.GetProgramState();
  if (iLastState != state)
    iLcd.clear();
  iLastState = state;
  
  if (state != Thermocycler::EOff) {
    //heat/cool status
    iLcd.setCursor(0, 1);
    iLcd.print("SNParty");
    
    char floatStr[32];
    sprintFloat(floatStr, iThermocycler.GetPlateTemp(), 1);
    char buf[32];
    sprintf(buf, "%s C", floatStr);
   
    iLcd.setCursor(13, 0);
    iLcd.print(buf);
        
    //lid temp
    sprintf(buf, "Lid: %3d C", (int)iThermocycler.GetLidTemp());
    iLcd.setCursor(10, 2);
    iLcd.print(buf);
   
    //state
    char* stateStr;
    if (iThermocycler.Ramping()) {
      if (iThermocycler.GetThermalDirection() == Thermocycler::HEAT)
        stateStr = "Heating";
      else
        stateStr = "Cooling";
    } else {
      stateStr = iThermocycler.GetCurrentStep()->GetName();
    }
    iLcd.setCursor(0, 0);
    sprintf(buf, "%-13s", stateStr);
    iLcd.print(buf);

    if (state == Thermocycler::ERunning && !iThermocycler.GetCurrentStep()->IsFinal()) {
      //Cycle
      if (ipDisplayCycle != NULL) {
        iLcd.setCursor(0, 3);
        int numCycles = ipDisplayCycle->GetNumCycles();
        int currentCycle = ipDisplayCycle->GetCurrentCycle() > numCycles ? numCycles : ipDisplayCycle->GetCurrentCycle();
        sprintf(buf, "%d of %d", currentCycle, numCycles);
        iLcd.print(buf);
      }
     
      //Time Remaining
      iLcd.setCursor(11, 3);
      iLcd.print("ETA: 1:32");
      
    } else {
      iLcd.setCursor(0, 3);
      iLcd.print("*** Run Complete ***");
      
    }
    
  } else {
    iLcd.setCursor(6, 1);
    iLcd.print("OpenPCR");
    
    iLcd.setCursor(4, 2);
    iLcd.print("Powered Off");
  }
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
