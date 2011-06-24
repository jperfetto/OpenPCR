/*
 *	program.cpp - OpenPCR control software.
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
#include "program.h"

#include "display.h"

////////////////////////////////////////////////////////////////////
// Class Step
void Step::SetName(const char* szName) {
  strncpy(iName, szName, sizeof(iName));
  iName[sizeof(iName) - 1] = '\0';
}

void Step::Reset() {
  iStepReturned = false;
  iDuration = 0;
  iTemp = 0;
  iName[0] = '\0'; 
}

void Step::BeginIteration() {
  iStepReturned = false;
}

Step* Step::GetNextStep() {
  if (iStepReturned) {
    return NULL;
  } else {
    iStepReturned = true;
    return this;
  }
}

////////////////////////////////////////////////////////////////////
// Class Cycle
ProgramComponent* Cycle::GetComponent(int index) {
  return iComponents[index];
}

PcrStatus Cycle::AddComponent(ProgramComponent* pComponent) {
  if (iNumComponents >= MAX_CYCLE_ITEMS)
    return ETooManySteps;
  
  iComponents[iNumComponents++] = pComponent;
  return ESuccess;
}

void Cycle::Reset() {
  iNumComponents = 0;
  iNumCycles = 0;
  iCurrentCycle = 0;
  iCurrentComponent = 0;
}

// iteration
void Cycle::BeginIteration() {
  iCurrentCycle = 0;
  RestartCycle();
}

Step* Cycle::GetNextStep() {
  //check for next step of existing component
  Step* pNextStep = iComponents[iCurrentComponent]->GetNextStep();
  
  //advance to next component if current component done
  if (pNextStep == NULL && ++iCurrentComponent < iNumComponents)
    pNextStep = iComponents[iCurrentComponent]->GetNextStep(); //should never be NULL
  
  //advance to next cycle if current cycle done
  if (pNextStep == NULL && ++iCurrentCycle < iNumCycles) {
    RestartCycle();
  
    //return first component of that cycle
    pNextStep = iComponents[iCurrentComponent]->GetNextStep(); //should never be NULL
  }
      
  return pNextStep;
}

void Cycle::RestartCycle() {
  iCurrentComponent = 0;
  
  for (int i = 0; i < iNumComponents; i++)
    iComponents[i]->BeginIteration();
}
