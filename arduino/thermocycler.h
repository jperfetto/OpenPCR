/*
 *	thermocycler.h - OpenPCR control software.
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
  Step* GetCurrentStep() { return ipCurrentStep; }
  ThermalDirection GetThermalDirection() { return iThermalDirection; }
  boolean Ramping() { return iRamping; }
  int GetPeltierPwm() { return iPeltierPwm; }
  float GetPlateTemp() { return iPlateTemp; }
  float GetLidTemp() { return iLidTemp; }
  
  // control
  void SetProgram(Cycle* pProgram, Cycle* pDisplayCycle); //takes ownership
  void Stop();
  PcrStatus Start();
  
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
  float TableLookup(const unsigned long lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue);
    float TableLookup(const unsigned int lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue);
  
  
private:
  // constants
  static const int PLATE_TEMP_SENSOR_PIN = 0;
  
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
  unsigned long iCycleStartTime;
  boolean iRamping;
  
  // peltier control
  ThermalDirection iThermalDirection;
  int iPeltierPwm;
  SPid iPeltierPid;
  SPid iLidPid;
};

#endif
