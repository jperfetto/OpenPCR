#include "pcr_includes.h"
#include <LiquidCrystal.h>

#include "thermocycler.h"
#include "program.h"
#include <Wire.h>

Thermocycler* gpThermocycler = NULL;

void setup() {
  gpThermocycler = new Thermocycler();
  
  //create program
  Cycle* pMaster = new Cycle(1);
  Cycle* pMain = new Cycle(30);
  pMaster->AddComponent(new Step("Extract", 60, 65));
  pMaster->AddComponent(new Step("IDenaturing", 30, 95));
  pMain->AddComponent(new Step("Denaturing", 30, 95));
  pMain->AddComponent(new Step("Annealing", 60, 62.8));
  pMain->AddComponent(new Step("Extending", 90, 68));
  pMaster->AddComponent(pMain);
  pMaster->AddComponent(new Step("FExtending", 300, 68));
  pMaster->AddComponent(new Step("Holding", 0, 4));
  gpThermocycler->SetProgram(pMaster, pMain);
  delay(1000);
  
  gpThermocycler->Start();
}

void loop() {
  gpThermocycler->Loop();
}

