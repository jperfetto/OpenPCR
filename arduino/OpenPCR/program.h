/*
 *	program.h - OpenPCR control software.
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

class Step;

////////////////////////////////////////////////////////////////////
// Class ProgramComponent
class ProgramComponent {
public:
  enum TType {
    EStep = 0,
    ECycle
  };
  
  virtual ~ProgramComponent() {}
  virtual TType GetType() = 0;
  
  // iteration
  virtual void BeginIteration() = 0;
  virtual Step* GetNextStep() = 0;
};

////////////////////////////////////////////////////////////////////
// Class Step
class Step: public ProgramComponent {
public:
  Step(char* name, int duration, float temp);
  
  // accessors
  char* GetName() { return iName; }
  int GetDuration() { return iDuration; }
  float GetTemp() { return iTemp; }
  void SetTemp(float temp) { iTemp = temp; } //TEMP HACK
  virtual TType GetType() { return EStep; }
  boolean IsFinal() { return iDuration == 0; }
  
  // iteration
  virtual void BeginIteration();
  virtual Step* GetNextStep();

private:
  int iDuration; //in seconds
  float iTemp; // C
  boolean iStepReturned;
  char iName[STEP_NAME_LENGTH];
};

////////////////////////////////////////////////////////////////////
// Class Cycle
class Cycle: public ProgramComponent {
public:
  Cycle(int numCycles);
  virtual ~Cycle();
  
  // accessors
  virtual TType GetType() { return ECycle; }
  int GetCurrentCycle() { return iCurrentCycle + 1; } //add 1 because cycles start at 0
  int GetNumCycles() { return iNumCycles; }
  
  // mutators
  PcrStatus AddComponent(ProgramComponent* pComponent); //takes ownership
  
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

