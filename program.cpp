#include "pcr_includes.h"
#include "program.h"

////////////////////////////////////////////////////////////////////
// Class Step
Step::Step(char* name, int duration, float temp):
  iDuration(duration),
  iTemp(temp),
  iStepReturned(false) {
    
  strncpy(iName, name, STEP_NAME_LENGTH - 1);
  iName[STEP_NAME_LENGTH] = '\0';
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
Cycle::Cycle(int numCycles):
  iNumComponents(0),
  iNumCycles(numCycles),
  iCurrentCycle(0),
  iCurrentComponent(0) {
}

Cycle::~Cycle() {
  for (int i = 0; i < iNumComponents; i++)
    delete iComponents[i];
}

PcrStatus Cycle::AddComponent(ProgramComponent* pComponent) {
  if (iNumComponents >= MAX_CYCLE_ITEMS)
    return ETooManySteps;
    
  iComponents[iNumComponents++] = pComponent;
  return ESuccess;
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

