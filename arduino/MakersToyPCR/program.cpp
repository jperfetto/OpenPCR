/*
 *  program.cpp - OpenPCR control software.
 *  Copyright (C) 2010-2012 Josh Perfetto. All Rights Reserved.
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

#include <EEPROM.h>

#include "display.h"

////////////////////////////////////////////////////////////////////
// Class Step
void Step::SetName(const char* szName) {
  strncpy(iName, szName, sizeof(iName));
  iName[sizeof(iName) - 1] = '\0';
}

void Step::Reset() {
  iStepReturned = false;
  iStepDurationS = 0;
  iRampDurationS = 0;
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

////////////////////////////////////////////////////////////////////
// Class CommandParser
void CommandParser::ParseCommand(SCommand& command, char* pCommandBuf) {
  char* pValue;
  memset(&command, NULL, sizeof(command));
  char buf[32];

  gpThermocycler->Stop(); //need to stop here to reset program pools
    
  char* pParam = strtok(pCommandBuf, "&");
  while (pParam) {  
    pValue = strchr(pParam, '=');
    *pValue++ = '\0';
    AddComponent(&command, pParam[0], pValue);
    pParam = strtok(NULL, "&");
  }
}

void CommandParser::AddComponent(SCommand* pCommand, char key, char* szValue) {
  switch(key) {
  case 'n':
    strncpy(pCommand->name, szValue, sizeof(pCommand->name) - 1);
    pCommand->name[sizeof(pCommand->name) - 1] = '\0';
    break;
  case 'c':
    if (strcmp(szValue, "start") == 0)
      pCommand->command = SCommand::EStart;
    else if (strcmp(szValue, "stop") == 0)
      pCommand->command = SCommand::EStop;
    else if (strcmp(szValue, "cfg") == 0)
      pCommand->command = SCommand::EConfig;
    break;
  case 'l':
    pCommand->lidTemp = atoi(szValue);
    break;
  case 'o':
    pCommand->contrast = atoi(szValue);
  case 'd':
    pCommand->commandId = atoi(szValue);
    break;
  case 'p':
    pCommand->pProgram = ParseProgram(szValue);
    break;
  }
}

Cycle* CommandParser::ParseProgram(char* pBuffer) {
  Cycle* pProgram = gpThermocycler->GetCyclePool().AllocateComponent();
  pProgram->SetNumCycles(1);
	
  char* pCycBuf = strtok(pBuffer, "()");
  while (pCycBuf != NULL) {
    pProgram->AddComponent(ParseCycle(pCycBuf));
    pCycBuf = strtok(NULL, "()");
  }
  
  return pProgram;
}

ProgramComponent* CommandParser::ParseCycle(char* pBuffer) {
  char countBuf[5];
	
  //find first step
  char* pStep = strchr(pBuffer, '[');
	
  //get cycle count
  int countLen = pStep - pBuffer;
  strncpy(countBuf, pBuffer, countLen);
  countBuf[countLen] = '\0';
  int cycCount = atoi(countBuf);
  
  Cycle* pCycle = gpThermocycler->GetCyclePool().AllocateComponent();
  pCycle->SetNumCycles(cycCount);
	
  //add steps
  while (pStep != NULL) {
    *pStep++ = '\0';
    char* pStepEnd = strchr(pStep, ']');
    *pStepEnd++ = '\0';

    Step* pNewStep = ParseStep(pStep);
    pCycle->AddComponent(pNewStep);
    pStep = strchr(pStepEnd, '[');
  }

  return pCycle;
}

Step* CommandParser::ParseStep(char* pBuffer) {
  //parse temp
  char* pTemp = strchr(pBuffer, '|');
  *pTemp++ = '\0';
  
  //parse name
  char* pName = strchr(pTemp, '|');
  *pName++ = '\0';
  
  //parse ramp duration if exists
  char* pEnd;
  char* pRampDuration = strchr(pName, '|');
  if (pRampDuration) {
    *pRampDuration++ = '\0';
    pEnd = strchr(pRampDuration, ']');
  } else {
    pEnd = strchr(pName, '|');
  }
  *pEnd = '\0';
	
  unsigned long stepDuration = atol(pBuffer);
  unsigned long rampDuration = pRampDuration == NULL ? 0 : atol(pRampDuration);
  float temp = atof(pTemp);

  Step* pStep = gpThermocycler->GetStepPool().AllocateComponent();
  
  pStep->SetName(pName);
  pStep->SetStepDurationS(stepDuration);
  pStep->SetRampDurationS(rampDuration);
  pStep->SetTemp(temp);
  return pStep;
}


////////////////////////////////////////////////////////////////////
// Class ProgramStore
//
// Note: Byte 0 of EEPROM is used for contrast
//       Bytes 1 and onwards are used for stored program string
//
uint8_t ProgramStore::RetrieveContrast() {
  return EEPROM.read(0);
}

#define PROG_START_STR "&c=start"
const char PROG_START_STR_P[] PROGMEM = PROG_START_STR;
boolean ProgramStore::RetrieveProgram(SCommand& command, char* pBuffer) {
  for (int i = 0; i < MAX_COMMAND_SIZE; i++)
    pBuffer[i] = EEPROM.read(i + 1);
  
  if (strncmp_P(pBuffer, PROG_START_STR_P, strlen(PROG_START_STR)) == 0) {
    //previous program stored
    CommandParser::ParseCommand(command, pBuffer);   
    return true;
    
  } else {
    return false;
  }
}



void ProgramStore::StoreContrast(uint8_t contrast) {
  EEPROM.write(0, contrast);
}

void ProgramStore::StoreProgram(const char* szProgram) {
  for (int i = 0; i < MAX_COMMAND_SIZE; i++)
    EEPROM.write(i + 1, szProgram[i]);
}


