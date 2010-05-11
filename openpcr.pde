#include "pcr_includes.h"
#include <LiquidCrystal.h>

#include "thermocycler.h"
#include "program.h"
#include <Wire.h>

Thermocycler* gpThermocycler = NULL;

void setup() {
  gpThermocycler = new Thermocycler();
  
  //create program
  Cycle* pCycle = new Cycle(20);
  pCycle->AddComponent(new Step("Low", 45, 50));
  pCycle->AddComponent(new Step("High", 45, 80));
  gpThermocycler->SetProgram(pCycle, pCycle);
  delay(1000);
  gpThermocycler->Start();
}

void loop() {
  gpThermocycler->Loop();
 // analogWrite(3, 127);
//  delay(250);
  delay(50);
}

