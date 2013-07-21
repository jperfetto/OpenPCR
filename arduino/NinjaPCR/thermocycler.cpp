/*
 *  thermocycler.cpp - OpenPCR control software.
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
#include "thermocycler.h"

#include "display.h"
#include "program.h"
#include "serialcontrol_chrome.h"
#include "../Wire/Wire.h"
#include <avr/pgmspace.h>

//constants

// I2C address for MCP3422 - base address for MCP3424
#define MCP3422_ADDRESS 0X68
#define MCP342X_RES_FIELD  0X0C // resolution/rate field
#define MCP342X_18_BIT     0X0C // 18-bit 3.75 SPS
#define MCP342X_BUSY       0X80 // read: output not ready

#define CYCLE_START_TOLERANCE 0.2
#define LID_START_TOLERANCE 1.0
//#define CYCLE_START_TOLERANCE 8.0
//#define LID_START_TOLERANCE 12.0

#define PLATE_PID_INC_NORM_P 1000
#define PLATE_PID_INC_NORM_I 250
#define PLATE_PID_INC_NORM_D 250

#define PLATE_PID_INC_LOW_THRESHOLD 40
#define PLATE_PID_INC_LOW_P 600
#define PLATE_PID_INC_LOW_I 200
#define PLATE_PID_INC_LOW_D 400

#define PLATE_PID_DEC_HIGH_THRESHOLD 70
#define PLATE_PID_DEC_HIGH_P 800
#define PLATE_PID_DEC_HIGH_I 700
#define PLATE_PID_DEC_HIGH_D 300

#define PLATE_PID_DEC_NORM_P 500
#define PLATE_PID_DEC_NORM_I 400
#define PLATE_PID_DEC_NORM_D 200

#define PLATE_PID_DEC_LOW_THRESHOLD 35
#define PLATE_PID_DEC_LOW_P 2000
#define PLATE_PID_DEC_LOW_I 100
#define PLATE_PID_DEC_LOW_D 200

#define PLATE_BANGBANG_THRESHOLD 2.0

#define MIN_PELTIER_PWM -1023
#define MAX_PELTIER_PWM 1023

#define MAX_LID_PWM 255
#define MIN_LID_PWM 0

#define STARTUP_DELAY 4000

//pid parameters
const SPIDTuning LID_PID_GAIN_SCHEDULE[] = {
  //maxTemp, kP, kI, kD
  { 
    70, 40, 0.15, 60   }
  ,
  { 
    200, 80, 1.1, 10   }
};

//public
Thermocycler::Thermocycler(boolean restarted):
iRestarted(restarted),
ipDisplay(NULL),
ipProgram(NULL),
ipDisplayCycle(NULL),
ipSerialControl(NULL),
iProgramState(EStartup),
ipPreviousStep(NULL),
ipCurrentStep(NULL),
iThermalDirection(OFF),
iPeltierPwm(0),
iCycleStartTime(0),
iRamping(true),
iPlatePid(&iPlateThermistor.GetTemp(), &iPeltierPwm, &iTargetPlateTemp, PLATE_PID_INC_NORM_P, PLATE_PID_INC_NORM_I, PLATE_PID_INC_NORM_D, DIRECT),
iLidPid(LID_PID_GAIN_SCHEDULE, MIN_LID_PWM, MAX_LID_PWM),
iTargetLidTemp(0) {

  ipDisplay = new Display();
  ipSerialControl = new SerialControl(ipDisplay);

  //init pins
  pinMode(15, INPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(9, OUTPUT);

  // SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (fastest)
  int clr;
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<4);
  clr=SPSR;
  clr=SPDR;
  delay(10); 

//#define ATMEGA328
#define LEONARDO

  iPlatePid.SetOutputLimits(MIN_PELTIER_PWM, MAX_PELTIER_PWM);

TCCR1A |= (1<<WGM11) | (1<<WGM10);
  // Peltier PWM
#ifdef LEONARDO
  TCCR1B = _BV(CS11);
  // Lid PWM
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(CS12);
#else
  TCCR1B = _BV(CS21);
  // Lid PWM
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS22);
#endif
  iszProgName[0] = '\0';
}

Thermocycler::~Thermocycler() {
  delete ipSerialControl;
  delete ipDisplay;
}

// accessors
int Thermocycler::GetNumCycles() {
  return ipDisplayCycle->GetNumCycles();
}

int Thermocycler::GetCurrentCycleNum() {
  int numCycles = GetNumCycles();
  return ipDisplayCycle->GetCurrentCycle() > numCycles ? numCycles : ipDisplayCycle->GetCurrentCycle();
}

Thermocycler::ThermalState Thermocycler::GetThermalState() {
  if (iProgramState == EStartup || iProgramState == EStopped)
    return EIdle;

  if (iRamping) {
    if (ipPreviousStep != NULL) {
      return ipCurrentStep->GetTemp() > ipPreviousStep->GetTemp() ? EHeating : ECooling;
    } 
    else {
      return iThermalDirection == HEAT ? EHeating : ECooling;
    }
  } 
  else {
    return EHolding;
  }
}

// control
void Thermocycler::SetProgram(Cycle* pProgram, Cycle* pDisplayCycle, const char* szProgName, int lidTemp) {
  Stop();

  ipProgram = pProgram;
  ipDisplayCycle = pDisplayCycle;

  strcpy(iszProgName, szProgName);
  iTargetLidTemp = lidTemp;
}

void Thermocycler::Stop() {
  iProgramState = EStopped;

  ipProgram = NULL;
  ipPreviousStep = NULL;
  ipCurrentStep = NULL;

  iStepPool.ResetPool();
  iCyclePool.ResetPool();

  ipDisplay->Clear();
}

PcrStatus Thermocycler::Start() {
	//Serial.print("st");
  if (ipProgram == NULL) {
	//Serial.print("nu");
    return ENoProgram;
  }
	//Serial.print("ok");

  //advance to lid wait state
  iProgramState = ELidWait;

  return ESuccess;
}

// internal
void Thermocycler::Loop() {
	  Serial.print(iProgramState);
  switch (iProgramState) {
  case EStartup:
	Serial.print(millis() > STARTUP_DELAY);
    if (millis() > STARTUP_DELAY) {
      iProgramState = EStopped;
      if (!iRestarted && !ipSerialControl->CommandReceived()) {
        //check for stored program
        SCommand command;
        if (ProgramStore::RetrieveProgram(command, (char*)ipSerialControl->GetBuffer()))
          ProcessCommand(command);
      }
    }
    break;

  case ELidWait:
    if (GetLidTemp() >= iTargetLidTemp - LID_START_TOLERANCE) {
      //lid has warmed, begin program
      iThermalDirection = OFF;
      iPeltierPwm = 0;
      PreprocessProgram();
      iProgramState = ERunning;

      ipProgram->BeginIteration();
      AdvanceToNextStep();

      iProgramStartTimeMs = millis();
    }
    break;

  case ERunning:
    //update program
    if (iProgramState == ERunning) {
      if (iRamping && abs(ipCurrentStep->GetTemp() - GetPlateTemp()) <= CYCLE_START_TOLERANCE && GetRampElapsedTimeMs() > ipCurrentStep->GetRampDurationS() * 1000) {
        //begin step hold
        //eta updates
        if (ipCurrentStep->GetRampDurationS() == 0) {
          //fast ramp
          iElapsedFastRampDegrees += absf(GetPlateTemp() - iRampStartTemp);
          iTotalElapsedFastRampDurationMs += millis() - iRampStartTime;
        }

        if (iRampStartTemp > GetPlateTemp()) {
          iHasCooled = true;
        }
        iRamping = false;
        iCycleStartTime = millis();

      } 
      else if (!iRamping && !ipCurrentStep->IsFinal() && millis() - iCycleStartTime > (unsigned long)ipCurrentStep->GetStepDurationS() * 1000) {
        //begin next step
        AdvanceToNextStep();

        //check for program completion
        if (ipCurrentStep == NULL || ipCurrentStep->IsFinal()) {
          iProgramState = EComplete;        
        }
      }
    }
    break;

  case EComplete:
    if (iRamping && ipCurrentStep != NULL && abs(ipCurrentStep->GetTemp() - GetPlateTemp()) <= CYCLE_START_TOLERANCE)
      iRamping = false;
    break;
  }
  //lid 
  iLidThermistor.ReadTemp();
  ControlLid();

  //plate  
  iPlateThermistor.ReadTemp();
  CalcPlateTarget();
  ControlPeltier();

  //program
  UpdateEta();
  ipDisplay->Update();
  ipSerialControl->Process();
}

//private
void Thermocycler::AdvanceToNextStep() {
  ipPreviousStep = ipCurrentStep;
  ipCurrentStep = ipProgram->GetNextStep();
  if (ipCurrentStep == NULL)
    return;

  //update eta calc params
  if (ipPreviousStep == NULL || ipPreviousStep->GetTemp() != ipCurrentStep->GetTemp()) {
    iRamping = true;
    iRampStartTime = millis();
    iRampStartTemp = GetPlateTemp();
  } 
  else {
    iCycleStartTime = millis(); //next step starts immediately
  }

  CalcPlateTarget();
  SetPlateControlStrategy();
}

void Thermocycler::SetPlateControlStrategy() {
  if (InControlledRamp())
    return;

  if (absf(iTargetPlateTemp - GetPlateTemp()) >= PLATE_BANGBANG_THRESHOLD && !InControlledRamp()) {
    iPlateControlMode = EBangBang;
    iPlatePid.SetMode(MANUAL);
  } 
  else {
    iPlateControlMode = EPIDPlate;
    iPlatePid.SetMode(AUTOMATIC);
  }

  if (iRamping) {
    if (iTargetPlateTemp >= GetPlateTemp()) {
      iDecreasing = false;
      if (iTargetPlateTemp < PLATE_PID_INC_LOW_THRESHOLD)
        iPlatePid.SetTunings(PLATE_PID_INC_LOW_P, PLATE_PID_INC_LOW_I, PLATE_PID_INC_LOW_D);
      else
        iPlatePid.SetTunings(PLATE_PID_INC_NORM_P, PLATE_PID_INC_NORM_I, PLATE_PID_INC_NORM_D);

    } 
    else {
      iDecreasing = true;
      if (iTargetPlateTemp > PLATE_PID_DEC_HIGH_THRESHOLD)
        iPlatePid.SetTunings(PLATE_PID_DEC_HIGH_P, PLATE_PID_DEC_HIGH_I, PLATE_PID_DEC_HIGH_D);
      else if (iTargetPlateTemp < PLATE_PID_DEC_LOW_THRESHOLD)
        iPlatePid.SetTunings(PLATE_PID_DEC_LOW_P, PLATE_PID_DEC_LOW_I, PLATE_PID_DEC_LOW_D);
      else
        iPlatePid.SetTunings(PLATE_PID_DEC_NORM_P, PLATE_PID_DEC_NORM_I, PLATE_PID_DEC_NORM_D);
    }
  }
}

void Thermocycler::CalcPlateTarget() {
  if (ipCurrentStep == NULL)
    return;

  if (InControlledRamp()) {
    //controlled ramp
    double tempDelta = ipCurrentStep->GetTemp() - ipPreviousStep->GetTemp();
    double rampPoint = (double)GetRampElapsedTimeMs() / (ipCurrentStep->GetRampDurationS() * 1000);
    iTargetPlateTemp = ipPreviousStep->GetTemp() + tempDelta * rampPoint;

  } 
  else {
    //fast ramp
    iTargetPlateTemp = ipCurrentStep->GetTemp();
  }
}

void Thermocycler::ControlPeltier() {
  ThermalDirection newDirection = OFF;

  if (iProgramState == ERunning || (iProgramState == EComplete && ipCurrentStep != NULL)) {
    // Check whether we are nearing target and should switch to PID control
    if (iPlateControlMode == EBangBang && absf(iTargetPlateTemp - GetPlateTemp()) < PLATE_BANGBANG_THRESHOLD) {
      iPlateControlMode = EPIDPlate;
      iPlatePid.SetMode(AUTOMATIC);
      iPlatePid.ResetI();
    }

    // Apply control mode
    if (iPlateControlMode == EBangBang)
      iPeltierPwm = iTargetPlateTemp > GetPlateTemp() ? MAX_PELTIER_PWM : MIN_PELTIER_PWM;
    iPlatePid.Compute();

    if (iDecreasing && iTargetPlateTemp > PLATE_PID_DEC_LOW_THRESHOLD) {
      if (iTargetPlateTemp < GetPlateTemp())
        iPlatePid.ResetI();
      else
        iDecreasing = false;
    } 

    if (iPeltierPwm > 0)
      newDirection = HEAT;
    else if (iPeltierPwm < 0)
      newDirection = COOL; 
    else
      newDirection = OFF;
  } 
  else {
    iPeltierPwm = 0;
  }

  iThermalDirection = newDirection;
  SetPeltier(newDirection, abs(iPeltierPwm));
}
void Thermocycler::ControlLid() {
  int drive = 0;  
  if (iProgramState == ERunning || iProgramState == ELidWait)
    drive = iLidPid.Compute(iTargetLidTemp, GetLidTemp());
  analogWrite(3, drive);
  analogValueLid = drive;
}

//PreprocessProgram initializes ETA parameters and validates/modifies ramp conditions
void Thermocycler::PreprocessProgram() {
  Step* pCurrentStep;
  Step* pPreviousStep = NULL;

  iProgramHoldDurationS = 0;
  iEstimatedTimeRemainingS = 0;
  iHasCooled = false;

  iProgramControlledRampDurationS = 0;
  iProgramFastRampDegrees = 0;
  iElapsedFastRampDegrees = 0;
  iTotalElapsedFastRampDurationMs = 0;  

  ipProgram->BeginIteration();
  while ((pCurrentStep = ipProgram->GetNextStep()) && !pCurrentStep->IsFinal()) {
    //validate ramp
    if (pPreviousStep != NULL && pCurrentStep->GetRampDurationS() * 1000 < absf(pCurrentStep->GetTemp() - pPreviousStep->GetTemp()) * PLATE_FAST_RAMP_THRESHOLD_MS) {
      //cannot ramp that fast, ignored set ramp
      pCurrentStep->SetRampDurationS(0);
    }

    //update eta hold
    iProgramHoldDurationS += pCurrentStep->GetStepDurationS();

    //update eta ramp
    if (pCurrentStep->GetRampDurationS() > 0) {
      //controlled ramp
      iProgramControlledRampDurationS += pCurrentStep->GetRampDurationS();
    } 
    else {
      //fast ramp
      double previousTemp = pPreviousStep ? pPreviousStep->GetTemp() : GetPlateTemp();
      iProgramFastRampDegrees += absf(previousTemp - pCurrentStep->GetTemp()) - CYCLE_START_TOLERANCE;
    }

    pPreviousStep = pCurrentStep;
  }
}

void Thermocycler::UpdateEta() {
  if (iProgramState == ERunning) {
    double fastSecondPerDegree;
    if (iElapsedFastRampDegrees == 0 || !iHasCooled)
      fastSecondPerDegree = 1.0;
    else
      fastSecondPerDegree = iTotalElapsedFastRampDurationMs / 1000 / iElapsedFastRampDegrees;

    unsigned long estimatedDurationS = iProgramHoldDurationS + iProgramControlledRampDurationS + iProgramFastRampDegrees * fastSecondPerDegree;
    unsigned long elapsedTimeS = GetElapsedTimeS();
    iEstimatedTimeRemainingS = estimatedDurationS > elapsedTimeS ? estimatedDurationS - elapsedTimeS : 0;
  }
}

void Thermocycler::SetPeltier(ThermalDirection dir, int pwm) {
  if (dir == COOL) {
    digitalWrite(2, HIGH);
    digitalWrite(4, LOW);
  } 
  else if (dir == HEAT) {
    digitalWrite(2, LOW);
    digitalWrite(4, HIGH);
  } 
  else {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
  }

  analogWrite(9, pwm);
  analogValuePeltier = (dir==COOL)?-pwm:pwm;
}

void Thermocycler::ProcessCommand(SCommand& command) {
  if (command.command == SCommand::EStart) {
    //find display cycle
    Cycle* pProgram = command.pProgram;
    Cycle* pDisplayCycle = pProgram;
    int largestCycleCount = 0;

    for (int i = 0; i < pProgram->GetNumComponents(); i++) {
      ProgramComponent* pComp = pProgram->GetComponent(i);
      if (pComp->GetType() == ProgramComponent::ECycle) {
        Cycle* pCycle = (Cycle*)pComp;
        if (pCycle->GetNumCycles() > largestCycleCount) {
          largestCycleCount = pCycle->GetNumCycles();
          pDisplayCycle = pCycle;
        }
      }
    }
    GetThermocycler().SetProgram(pProgram, pDisplayCycle, command.name, command.lidTemp);
    GetThermocycler().Start();

  } 
  else if (command.command == SCommand::EStop) {
    GetThermocycler().Stop(); //redundant as we already stopped during parsing

  } 
  else if (command.command == SCommand::EConfig) {
    //update displayed
    ipDisplay->SetContrast(command.contrast);

    //update stored contrast
    ProgramStore::StoreContrast(command.contrast);
  }
}

