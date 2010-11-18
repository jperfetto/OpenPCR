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

