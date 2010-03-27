#include "pcr_includes.h"
#include <LiquidCrystal.h>

#include "thermocycler.h"
#include "program.h"

Thermocycler* gpThermocycler = NULL;

void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  gpThermocycler = new Thermocycler();
  
  //create program
  Cycle* pCycle = new Cycle(20);
  pCycle->AddComponent(new Step("Low", 45, 50));
  pCycle->AddComponent(new Step("High", 45, 80));
  gpThermocycler->SetProgram(pCycle, pCycle);
  gpThermocycler->Start();
}

void loop() {
  gpThermocycler->Loop();
 // analogWrite(3, 127);
 // delay(250);
}
