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
  
  enum ThermalDirection {
    OFF,
    HEAT,
    COOL
  };
  
  Thermocycler();
  ~Thermocycler();
  
  // accessors
  ProgramState GetProgramState() { return iProgramState; }
  ThermalState GetThermalState() { return iThermalState; }
  ThermalDirection GetThermalDirection() { return iThermalDirection; }
  int GetPeltierPwm() { return iPeltierPwm; }
  float GetPlateTemp() { return iPlateTemp; }
  
  // control
  void SetProgram(Cycle* pProgram, Cycle* pDisplayCycle); //takes ownership
  void Stop();
  PcrStatus Start();
  
  // internal
  void Loop();
  
private:
  void ReadPlateTemp();
  void ControlPeltier();
 
  //util functions
  void SetPeltier(ThermalDirection dir, int pwm);
  
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
  
  // peltier control
  ThermalDirection iThermalDirection;
  int iPeltierPwm;
};

#endif
