/*
 *  display.cpp - OpenPCR control software.
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
#include "display.h"

#include "thermocycler.h"
#include "thermistors.h"
#include "program.h"

#define RESET_INTERVAL 30000 //ms

//progmem strings
const char HEATING_STR[] PROGMEM = "Heating";
const char COOLING_STR[] PROGMEM = "Cooling";
const char LIDWAIT_STR[] PROGMEM = "Heating Lid";
const char STOPPED_STR[] PROGMEM = "Ready";
const char RUN_COMPLETE_STR[] PROGMEM = "*** Run Complete ***";
const char OPENPCR_STR[] PROGMEM = "OpenPCR";
const char POWERED_OFF_STR[] PROGMEM = "Powered Off";
const char ETA_OVER_1000H_STR[] PROGMEM = "ETA: >1000h";

const char LID_FORM_STR[] PROGMEM = "Lid: %3d C";
const char CYCLE_FORM_STR[] PROGMEM = "%d of %d";
const char ETA_HOURMIN_FORM_STR[] PROGMEM = "ETA: %d:%02d";
const char ETA_SEC_FORM_STR[] PROGMEM = "ETA:  %2ds";
const char BLOCK_TEMP_FORM_STR[] PROGMEM = "%s C";
const char STATE_FORM_STR[] PROGMEM = "%-13s";
const char VERSION_FORM_STR[] PROGMEM = "Firmware v%s";

Display::Display():
  iLcd(6, 7, 8, A5, 16, 17),
  iLastState(Thermocycler::EStartup) {

  iLcd.begin(20, 4);
  iLastReset = millis();
#ifdef DEBUG_DISPLAY
  iszDebugMsg[0] = '\0';
#endif
  
  // Set contrast
  iContrast = ProgramStore::RetrieveContrast();
  analogWrite(5, iContrast);
}

void Display::Clear() {
  iLastState = Thermocycler::EClear;
}

void Display::SetContrast(uint8_t contrast) {
  iContrast = contrast;
  analogWrite(5, iContrast);
  iLcd.begin(20, 4);
}
  
void Display::SetDebugMsg(char* szDebugMsg) {
#ifdef DEBUG_DISPLAY
  strcpy(iszDebugMsg, szDebugMsg);
#endif
  iLcd.clear();
  Update();
}

void Display::Update() {
  char buf[16];
  
  Thermocycler::ProgramState state = GetThermocycler().GetProgramState();
  if (iLastState != state)
    iLcd.clear();
  iLastState = state;
  
  // check for reset
  if (millis() - iLastReset > RESET_INTERVAL) {  
    iLcd.begin(20, 4);
    iLastReset = millis();
  }
  
  switch (state) {
  case Thermocycler::ERunning:
  case Thermocycler::EComplete:
  case Thermocycler::ELidWait:
  case Thermocycler::EStopped:
    iLcd.setCursor(0, 1);
 #ifdef DEBUG_DISPLAY
    iLcd.print(iszDebugMsg);
 #else
    iLcd.print(GetThermocycler().GetProgName());
 #endif
           
    DisplayLidTemp();
    DisplayBlockTemp();
    DisplayState();

    if (state == Thermocycler::ERunning && !GetThermocycler().GetCurrentStep()->IsFinal()) {
      DisplayCycle();
      DisplayEta();
    } else if (state == Thermocycler::EComplete) {
      iLcd.setCursor(0, 3);
      iLcd.print(rps(RUN_COMPLETE_STR));
    }
    break;
  
  case Thermocycler::EStartup:
    iLcd.setCursor(6, 1);
    iLcd.print(rps(OPENPCR_STR));

      iLcd.setCursor(2, 2);
      sprintf_P(buf, VERSION_FORM_STR, OPENPCR_FIRMWARE_VERSION_STRING);
      iLcd.print(buf);
    break;
  }
}

void Display::DisplayEta() {
  char timeString[16];
  unsigned long timeRemaining = GetThermocycler().GetTimeRemainingS();
  int hours = timeRemaining / 3600;
  int mins = (timeRemaining % 3600) / 60;
  int secs = timeRemaining % 60;
  
  if (hours >= 1000)
    strcpy_P(timeString, ETA_OVER_1000H_STR);
  else if (mins >= 1 || hours >= 1)
    sprintf_P(timeString, ETA_HOURMIN_FORM_STR, hours, mins);
  else
    sprintf_P(timeString, ETA_SEC_FORM_STR, secs);
  
  iLcd.setCursor(20 - strlen(timeString), 3);
  iLcd.print(timeString);
}

void Display::DisplayLidTemp() {
  char buf[16];
  sprintf_P(buf, LID_FORM_STR, (int)(GetThermocycler().GetLidTemp() + 0.5));

  iLcd.setCursor(10, 2);
  iLcd.print(buf);
}

void Display::DisplayBlockTemp() {
  char buf[16];
  char floatStr[16];
  
  sprintFloat(floatStr, GetThermocycler().GetPlateTemp(), 1, true);
  sprintf_P(buf, BLOCK_TEMP_FORM_STR, floatStr);
 
  iLcd.setCursor(13, 0);
  iLcd.print(buf);
}

void Display::DisplayCycle() {
  char buf[16];
  
  iLcd.setCursor(0, 3);
  sprintf_P(buf, CYCLE_FORM_STR, GetThermocycler().GetCurrentCycleNum(), GetThermocycler().GetNumCycles());
  iLcd.print(buf);
}

void Display::DisplayState() {
  char buf[32];
  char* stateStr;
  
  switch (GetThermocycler().GetProgramState()) {
  case Thermocycler::ELidWait:
    stateStr = rps(LIDWAIT_STR);
    break;
    
  case Thermocycler::ERunning:
  case Thermocycler::EComplete:
    switch (GetThermocycler().GetThermalState()) {
    case Thermocycler::EHeating:
      stateStr = rps(HEATING_STR);
      break;
    case Thermocycler::ECooling:
      stateStr = rps(COOLING_STR);
      break;
    case Thermocycler::EHolding:
      stateStr = GetThermocycler().GetCurrentStep()->GetName();
      break;
    case Thermocycler::EIdle:
      stateStr = rps(STOPPED_STR);
      break;
    }
    break;
    
  case Thermocycler::EStopped:
    stateStr = rps(STOPPED_STR);
    break;
  }
  
  iLcd.setCursor(0, 0);
  sprintf_P(buf, STATE_FORM_STR, stateStr);
  iLcd.print(buf);
}
