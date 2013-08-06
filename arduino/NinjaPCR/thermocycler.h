/*
 *  thermocycler.h - OpenPCR control software.
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

#ifndef _THERMOCYCLER_H_
#define _THERMOCYCLER_H_

#include "PID_v1.h"
#include "pid.h"
#include "program.h"
#include "thermistors.h"


class Display;
class SerialControl;

class Thermocycler {
public:
  enum ProgramState {
    EStartup = 0,
    EStopped,
    ELidWait,
    ERunning,
    EComplete,
    EError,
    EClear //for Display clearing only
  };
  
  enum ThermalState {
    EHolding = 0,
    EHeating,
    ECooling,
    EIdle
  };
  
  enum ThermalDirection {
    OFF,
    HEAT,
    COOL
  };
  
  enum ControlMode {
    EBangBang,
    EPIDLid,
    EPIDPlate
  };
  
  Thermocycler(boolean restarted);
  ~Thermocycler();
  
  // accessors
  ProgramState GetProgramState() { return iProgramState; }
  ThermalState GetThermalState();
  Step* GetCurrentStep() { return ipCurrentStep; }
  Cycle* GetDisplayCycle() { return ipDisplayCycle; }
  int GetNumCycles();
  int GetCurrentCycleNum();
  const char* GetProgName() { return iszProgName; }
  Display* GetDisplay() { return ipDisplay; }
  ProgramComponentPool<Cycle, 4>& GetCyclePool() { return iCyclePool; }
  ProgramComponentPool<Step, 20>& GetStepPool() { return iStepPool; }
  
  boolean Ramping() { return iRamping; }
  int GetPeltierPwm() { return iPeltierPwm; }
  double GetLidTemp() { return iLidThermistor.GetTemp(); }
  double GetPlateTemp() { return iPlateThermistor.GetTemp(); }
  double GetPlateResistance() { return iPlateThermistor.GetResistance(); }
  unsigned long GetTimeRemainingS() { return iEstimatedTimeRemainingS; }
  unsigned long GetElapsedTimeS() { return (millis() - iProgramStartTimeMs) / 1000; }
  unsigned long GetRampElapsedTimeMs() { return millis() - iRampStartTime; }
  boolean InControlledRamp() { return iRamping && ipCurrentStep->GetRampDurationS() > 0 && ipPreviousStep != NULL; }
  
  int getAnalogValuePeltier() { return analogValuePeltier; }
  int getAnalogValueLid() { return analogValueLid; }
  
  // control
  void SetProgram(Cycle* pProgram, Cycle* pDisplayCycle, const char* szProgName, int lidTemp); //takes ownership of cycles
  void Stop();
  PcrStatus Start();
  void ProcessCommand(SCommand& command);
  
  // internal
  void Loop();
  
private:
  void CheckPower();
  void ReadLidTemp();
  void ReadPlateTemp();
  void CalcPlateTarget();
  void ControlPeltier();
  void ControlLid();
  void PreprocessProgram();
  void UpdateEta();
 
  //util functions
  void AdvanceToNextStep();
  void SetPlateControlStrategy();
  void SetPeltier(ThermalDirection dir, int pwm);
  
private:
  // components
  Display* ipDisplay;
  SerialControl* ipSerialControl;
  CLidThermistor iLidThermistor;
  CPlateThermistor iPlateThermistor;
  ProgramComponentPool<Cycle, 4> iCyclePool;
  ProgramComponentPool<Step, 20> iStepPool;
  
  // state
  ProgramState iProgramState;
  double iTargetPlateTemp;
  double iTargetLidTemp;
  Cycle* ipProgram;
  Cycle* ipDisplayCycle;
  char iszProgName[21];
  Step* ipPreviousStep;
  Step* ipCurrentStep;
  unsigned long iCycleStartTime;
  boolean iRamping;
  boolean iDecreasing;
  boolean iRestarted;
  
  ControlMode iPlateControlMode;
  
  // peltier control
  PID iPlatePid;
  CPIDController iLidPid;
  ThermalDirection iThermalDirection; //holds actual real-time state
  double iPeltierPwm;
  
  // program eta calculation
  unsigned long iProgramStartTimeMs;
  unsigned long iProgramHoldDurationS;
  
  unsigned long iProgramControlledRampDurationS;
  double iProgramFastRampDegrees;
  double iElapsedFastRampDegrees;
  unsigned long iTotalElapsedFastRampDurationMs;
  
  double iRampStartTemp;
  unsigned long iRampStartTime;
  unsigned long iEstimatedTimeRemainingS;
  boolean iHasCooled;
  int analogValuePeltier;
  int analogValueLid;
};

#endif
