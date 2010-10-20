#include "pcr_includes.h"
#include <LiquidCrystal.h>

#include "thermocycler.h"
#include "program.h"
#include <Wire.h>

Thermocycler* gpThermocycler = NULL;

void setup() {
  gpThermocycler = new Thermocycler();
  
  //create program
  Cycle* pCycle = new Cycle(30);

  pCycle->AddComponent(new Step("Denaturing", 30, 95));
//  pCycle->AddComponent(new Step("Annealing", 30, 62));
//  pCycle->AddComponent(new Step("Extending", 30, 68));
  gpThermocycler->SetProgram(pCycle, pCycle);
  delay(1000);
  
  gpThermocycler->Start();
}

void loop() {
  gpThermocycler->Loop();
}

