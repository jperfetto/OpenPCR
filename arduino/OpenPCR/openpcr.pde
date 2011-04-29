/*
 *	openpcr.pde - OpenPCR control software.
 *  Copyright (C) 2010 Josh Perfetto. All Rights Reserved.
 *
 *  OpenPCR control software is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenPCR control software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  the OpenPCR control software.  If not, see <http://www.gnu.org/licenses/>.
 */

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
//  pMaster->AddComponent(new Step("Hold", 0, 55));
  pMaster->AddComponent(new Step("IDenaturing", 30, 95));
  Cycle* pMain = new Cycle(30);
  pMain->AddComponent(new Step("Denaturing", 30, 95));
  pMain->AddComponent(new Step("Annealing", 60, 55));
  pMain->AddComponent(new Step("Extending", 60, 72));
  pMaster->AddComponent(pMain);
  pMaster->AddComponent(new Step("FExtending", 300, 72));
  pMaster->AddComponent(new Step("Holding", 0, 4));
  
//
/*  Cycle* pMain = new Cycle(30);
  pMaster->AddComponent(new Step("Holding", 0, 90));
  pMaster->AddComponent(new Step("Holding 95", 45, 95));
//  pMaster->AddComponent(new Step("Testing", 1, 80));
  pMaster->AddComponent(new Step("PCR Good", 0, 20));
//  pMaster->AddComponent(new Step("Holding", 0, 95));
//  pMaster->AddComponent(new Step("Extract", 60, 65));
//  pMaster->AddComponent(new Step("IDenaturing", 300, 92));
  pMain->AddComponent(new Step("Denaturing", 30, 92));
  pMain->AddComponent(new Step("Annealing", 30, 53.8)); //62.8));
//  pMain->AddComponent(new Step("Annealing", 60, 57));
  pMain->AddComponent(new Step("Extending", 30, 69));
  pMaster->AddComponent(pMain);
  pMaster->AddComponent(new Step("FExtending", 300, 69));
  pMaster->AddComponent(new Step("Holding", 0, 4));
 */
  gpThermocycler->SetProgram(pMaster, pMain);
  delay(1000);
  
  gpThermocycler->Start();
}

void loop() {
  gpThermocycler->Loop();
}

