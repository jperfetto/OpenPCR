#ifndef _THERMOCYCLER_H_
#define _THERMOCYCLER_H_

class Display;
class Cycle;
class Step;

class Thermocycler {
public:
  enum ProgramState {
    EStopped = 0,
    ERunning,
    EFinished
  };
  
  enum ThermalState {
    EHolding = 0,
    EHeating,
    ECooling
  };
  
  Thermocycler();
  ~Thermocycler();
  
  // accessors
  ProgramState GetProgramState() { return iProgramState; }
  ThermalState GetThermalState() { return iThermalState; }
  float GetPlateTemp() { return iPlateTemp; }
  
  // control
  void SetProgram(Cycle* pProgram, Cycle* pDisplayCycle); //takes ownership
  void Stop();
  PcrStatus Start();
  
  // internal
  void Loop();
  
private:
  void ReadPlateTemp();
  
private:
  // constants
  static const int PLATE_TEMP_SENSOR_PIN = 0;
  
  // table identifies resistances in ohms for -20C to 100C
  static const unsigned long PLATE_RESISTANCE_TABLE[];
  
  // components
  Display* ipDisplay;
  
  // state
  ProgramState iProgramState;
  ThermalState iThermalState;
  float iPlateTemp;
  Cycle* ipProgram;
  Step* ipCurrentStep;
};

#endif
