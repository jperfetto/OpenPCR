#include "pcr_includes.h"
#include "display.h"

#include "thermocycler.h"
#include "program.h"

Display::Display(Thermocycler& thermocycler):
  iLcd(6, 7, 8, 9, 16, 17),
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
    iLcd.setCursor(6, 1);
    //iLcd.print("OpenPCR");
    
   // return;
    
    //heat/cool status
    iLcd.setCursor(0, 1);
    iLcd.print("SNP Genotyping");
    
    char floatStr[10];
    sprintFloat(floatStr, iThermocycler.GetPlateTemp(), 1);
    char buf[32];
    sprintf(buf, "%s C", floatStr);
   
    iLcd.setCursor(13, 0);
    iLcd.print(buf);
    
    //lid temp
    sprintf(buf, "Lid: %3d C", (int)iThermocycler.GetLidTemp());
    iLcd.setCursor(10, 2);
    iLcd.print(buf);
  
    iLcd.setCursor(0, 0);
    iLcd.print("Annealing");
   
    //Cycle
    if (ipDisplayCycle != NULL) {
      iLcd.setCursor(0, 3);
      sprintf(buf, "%d of %d", ipDisplayCycle->GetCurrentCycle(), 25);//***ipDisplayCycle->GetNumCycles());
      iLcd.print(buf);
    }
   
    //Time Remaining
    iLcd.setCursor(11, 3);
    iLcd.print("ETA: 1:32");
    
    //Debug
  //  sprintf(buf, "Time = %u . %d s", iThermocycler.GetRunTime() / 1000, (int)(iThermocycler.GetRunTime() % 1000));
  //  iLcd.setCursor(0, 2);
//    iLcd.print(buf);
    
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
