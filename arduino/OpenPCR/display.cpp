/*
 *	display.cpp - OpenPCR control software.
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

#include "pcr_includes.h"
#include "display.h"

#include "thermocycler.h"
#include "program.h"

#define RESET_INTERVAL 30000 //ms

// #define DEBUG_DISPLAY

Display::Display(Thermocycler& thermocycler):
  iLcd(6, 7, 8, A5, 16, 17),
  iThermocycler(thermocycler),
  iLastState(Thermocycler::EOff),
  iContrast(10) {

  iLcd.begin(20, 4);
  iLastReset = millis();
  iszDebugMsg[0] = '\0';
  
  // Set contrast
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
  strcpy(iszDebugMsg, szDebugMsg);
  iLcd.clear();
  Update();
}

const char LID_FORM_STR[] PROGMEM = "Lid: %3d C";
const char HEATING_STR[] PROGMEM = "Heating";
const char COOLING_STR[] PROGMEM = "Cooling";

void Display::Update() {
  char pbuf[32];
  Thermocycler::ProgramState state = iThermocycler.GetProgramState();
  if (iLastState != state)
    iLcd.clear();
  iLastState = state;
  
  // check for reset
  if (millis() - iLastReset > RESET_INTERVAL) {  
    iLcd.begin(20, 4);
    iLastReset = millis();
  }
  
  if (state == Thermocycler::ERunning || state == Thermocycler::EComplete) {
    //heat/cool status
    iLcd.setCursor(0, 1);
 #ifdef DEBUG_DISPLAY
    iLcd.print(iszDebugMsg);
 #else
   iLcd.print(iThermocycler.GetProgName());
 #endif
    
    char floatStr[32];
    sprintFloat(floatStr, iThermocycler.GetPlateTemp(), 1, true);
    char buf[32];
    sprintf(buf, "%s C", floatStr);
   
    iLcd.setCursor(13, 0);
    iLcd.print(buf);
        
    //lid temp
    strcpy_P(pbuf, LID_FORM_STR);
    sprintf(buf, pbuf, (int)(iThermocycler.GetLidTemp() + 0.5));
    iLcd.setCursor(10, 2);
    iLcd.print(buf);
   
    //state
    char* stateStr;
    if (iThermocycler.Ramping()) {
      if (iThermocycler.GetThermalDirection() == Thermocycler::HEAT)
        stateStr = rps(HEATING_STR);
      else
        stateStr = rps(COOLING_STR);
    } else {
      stateStr = iThermocycler.GetCurrentStep()->GetName();
    }
    iLcd.setCursor(0, 0);
    sprintf(buf, "%-13s", stateStr);
    iLcd.print(buf);

    if (state == Thermocycler::ERunning && !iThermocycler.GetCurrentStep()->IsFinal()) {
      //Cycle
      iLcd.setCursor(0, 3);
      sprintf(buf, "%d of %d", iThermocycler.GetCurrentCycleNum(), iThermocycler.GetNumCycles());
      iLcd.print(buf);
     
      //Time Remaining
      unsigned long timeRemaining = iThermocycler.GetTimeRemainingS();
      int hours = timeRemaining / 3600;
      int mins = (timeRemaining % 3600) / 60;
      int secs = timeRemaining % 60;
      char timeString[32];
      if (hours >= 10)
        strcpy(timeString, "ETA: >10h");
      else if (mins >= 1 || hours >= 1)
        sprintf(timeString, "ETA: %d:%02d", hours, mins);
      else
        sprintf(timeString, "ETA:  %2ds", secs);
      iLcd.setCursor(11, 3);
      iLcd.print(timeString);
      
    } else {
      iLcd.setCursor(0, 3);
      iLcd.print("*** Run Complete ***");
    }
    
  } else {
    iLcd.setCursor(6, 1);
    iLcd.print("OpenPCR");
    
    if (state == Thermocycler::EOff) {
      iLcd.setCursor(4, 2);
      iLcd.print("Powered Off");
    } else if (state == Thermocycler::EStopped) {
      iLcd.setCursor(4, 2);
      iLcd.print("Standing By");
    }
  }
}
