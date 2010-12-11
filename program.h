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

