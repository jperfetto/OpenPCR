/*
 *  openpcr.pde - OpenPCR control software.
 *  Copyright (C) 2010-2011 Josh Perfetto. All Rights Reserved.
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

#include <LiquidCrystal.h>
#include "pcr_includes.h"
#include "thermocycler.h"
#include "program.h"

Thermocycler* gpThermocycler = NULL;

void setup() {
  gpThermocycler = new Thermocycler();
  
/*
  //create test program
  Cycle* pMaster = new Cycle(1);
  pMaster->AddComponent(new Step("IDenaturing", 10, 95));
  Cycle* pMain = new Cycle(1);
  pMain->AddComponent(new Step("Denaturing", 10, 95));
  pMain->AddComponent(new Step("Annealing", 10, 55));
  pMain->AddComponent(new Step("Extending", 10, 72));
  pMaster->AddComponent(pMain);
  pMaster->AddComponent(new Step("FExtending", 10, 72));
  pMaster->AddComponent(new Step("Holding", 0, 50));
 
  gpThermocycler->SetProgram(pMaster, pMain, "Test Name", 110);
  delay(1000);  
  gpThermocycler->Start();
*/
}

void loop() {
  gpThermocycler->Loop();
}

