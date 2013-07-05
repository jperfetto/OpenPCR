/*
 *  program.h - OpenPCR control software.
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

#ifndef _PROGRAM_H_
#define _PROGRAM_H_

class Step;

////////////////////////////////////////////////////////////////////
// Class ProgramComponent
class ProgramComponent {
public:
  enum TType {
    EStep = 0,
    ECycle
  };
  
  virtual void Reset() = 0;
  virtual TType GetType() = 0;
  
  // iteration
  virtual void BeginIteration() = 0;
  virtual Step* GetNextStep() = 0;
};

////////////////////////////////////////////////////////////////////
// Class Step
class Step: public ProgramComponent {
public:  
  // accessors
  char* GetName() { return iName; }
  unsigned int GetStepDurationS() { return iStepDurationS; }
  unsigned long GetRampDurationS() { return iRampDurationS; }
  float GetTemp() { return iTemp; }
  virtual TType GetType() { return EStep; }
  boolean IsFinal() { return iStepDurationS == 0; }

  // mutators
  void SetStepDurationS(unsigned long stepDurationS) { iStepDurationS = stepDurationS; }
  void SetRampDurationS(unsigned long rampDurationS) { iRampDurationS = rampDurationS; }
  void SetTemp(float temp) { iTemp = temp; }
  void SetName(const char* szName);
  
  virtual void Reset();
  
  // iteration
  virtual void BeginIteration();
  virtual Step* GetNextStep();

private:
  unsigned int iStepDurationS; //in seconds
  unsigned long iRampDurationS; //in seconds, refers to ramp before the current step hold
  float iTemp; // C
  boolean iStepReturned;
  char iName[STEP_NAME_LENGTH];
};

////////////////////////////////////////////////////////////////////
// Class Cycle
class Cycle: public ProgramComponent {
public:
  // accessors
  virtual TType GetType() { return ECycle; }
  int GetCurrentCycle() { return iCurrentCycle + 1; } //add 1 because cycles start at 0
  int GetNumCycles() { return iNumCycles; }
  int GetNumComponents() { return iNumComponents; }
  ProgramComponent* GetComponent(int index);
  
  // mutators
  void SetNumCycles(int numCycles) { iNumCycles = numCycles; }
  PcrStatus AddComponent(ProgramComponent* pComponent); //takes ownership
  virtual void Reset();
  
  // iteration
  virtual void BeginIteration();
  virtual Step* GetNextStep();
  
private:
  void RestartCycle();

private:
  ProgramComponent* iComponents[MAX_CYCLE_ITEMS];
  int iNumComponents;
  int iNumCycles;
  
  int iCurrentCycle;
  int iCurrentComponent; // -1 means no component
};

////////////////////////////////////////////////////////////////////
// Class ProgramComponentPool
template <class T, int N>
class ProgramComponentPool {
public:
  ProgramComponentPool() { iAllocatedComponents = 0; }

  T* AllocateComponent() { 
    if (iAllocatedComponents == N)
      return NULL;
      
    T* pComponent = &iComponents[iAllocatedComponents++];
    pComponent->Reset();
    return pComponent;
  }
  
  void ResetPool() { iAllocatedComponents = 0; }
  
private:
  int iAllocatedComponents;
  T iComponents[N];
};

////////////////////////////////////////////////////////////////////
// Struct SCommand
struct SCommand {
  char name[21];
  uint16_t commandId;
  enum TCommandType {
    ENone = 0,
    EStart,
    EStop,
    EConfig
  } command;
  int lidTemp;
  uint8_t contrast;
  Cycle* pProgram;
};

////////////////////////////////////////////////////////////////////
// Class CommandParser
class CommandParser {
public:
  static void ParseCommand(SCommand& command, char* pCommandBuf);

private:
  static void AddComponent(SCommand* pCommand, char key, char* szValue);
  static Cycle* ParseProgram(char* pBuffer);
  static ProgramComponent* ParseCycle(char* pBuffer);
  static Step* ParseStep(char* pBuffer);
};

////////////////////////////////////////////////////////////////////
// Class ProgramStore
class ProgramStore {
public:
  //reading
  static uint8_t RetrieveContrast();
  static boolean RetrieveProgram(SCommand& command, char* pBuffer);

  //writing
  static void StoreContrast(uint8_t contrast);
  static void StoreProgram(const char* szProgram);
};
  

#endif
