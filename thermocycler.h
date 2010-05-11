#ifndef _THERMOCYCLER_H_
#define _THERMOCYCLER_H_

class Display;
class Cycle;
class Step;
class SerialControl;

class Thermocycler {
public:
  enum ProgramState {
    EOff = 0,
    EStopped,
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
  float GetLidTemp() { return iLidTemp; }
  
  // control
  void SetProgram(Cycle* pProgram, Cycle* pDisplayCycle); //takes ownership
  void Stop();
  PcrStatus Start();
  
  // temp
  unsigned long GetRunTime() { return iRunTime; }
  
  // internal
  void Loop();
  
private:
  void CheckPower();
  void ReadLidTemp();
  void ReadPlateTemp();
  void ControlPeltier();
  void ControlLid();
 
  //util functions
  void SetPeltier(ThermalDirection dir, int pwm);
  uint8_t mcp342xWrite(uint8_t config);
  uint8_t mcp342xRead(int32_t &data);
  
  
private:
  // constants
  static const int PLATE_TEMP_SENSOR_PIN = 0;
  
  // table identifies resistances in ohms for -20C to 100C
  static const unsigned long PLATE_RESISTANCE_TABLE[];
  
  // components
  Display* ipDisplay;
  SerialControl* ipSerialControl;
  
  // state
  ProgramState iProgramState;
  ThermalState iThermalState;
  float iPlateTemp;
  float iLidTemp;
  Cycle* ipProgram;
  Step* ipCurrentStep;
  
  // peltier control
  ThermalDirection iThermalDirection;
  int iPeltierPwm;
  SPid iPeltierPid;
  
  //temp
  unsigned long iRunTime;
  unsigned long iStartTime;
  boolean iRunning;
};

#endif
