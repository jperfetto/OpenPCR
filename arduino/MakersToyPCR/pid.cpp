 /*
 *  pid.cpp - OpenPCR control software.
 *  Copyright (C) 2012 Josh Perfetto. All Rights Reserved.
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
#include "pid.h"

////////////////////////////////////////////////////////////////////
// Class CPIDController
CPIDController::CPIDController(const SPIDTuning* pGainSchedule, int minOutput, int maxOutput):
  ipGainSchedule(pGainSchedule),
  iMinOutput(minOutput),
  iMaxOutput(maxOutput),
  iIntegrator(0),
  iPreviousError(0) {
}
//------------------------------------------------------------------------------
double CPIDController::Compute(double target, double currentValue) {
  //calc values for this computation
  const SPIDTuning* pPIDTuning = DetermineGainSchedule(target);
  double error = target - currentValue;
  
  //perform basic PID calculation
  double pTerm = error;
  double iTerm = iIntegrator + error;
  double dTerm = error - iPreviousError;
  double output = (pPIDTuning->kP * pTerm) + (pPIDTuning->kI * iTerm) + (pPIDTuning->kD * dTerm);  
  
  //reset integrator if pTerm maxed out in drivable direction
  if ((iMaxOutput && pTerm * pPIDTuning->kP > iMaxOutput) ||
      (iMinOutput && pTerm * pPIDTuning->kP < iMinOutput)) {
    iIntegrator = 0;
    
  //accumulate integrator if output not maxed out in drivable direction
  } else if ((iMinOutput == 0 || output > iMinOutput) && 
             (iMaxOutput == 0 || output < iMaxOutput)) {
    iIntegrator += error;
  }
  
  //latch integrator and output value to controllable range
  LatchValue(&iIntegrator, iMinOutput, iMaxOutput);
  LatchValue(&output, iMinOutput, iMaxOutput);

  //update values for next derivative computation
  iPreviousError = error;

  return output;
}
//------------------------------------------------------------------------------
const SPIDTuning* CPIDController::DetermineGainSchedule(double target) {
  const SPIDTuning* pGainScheduleItem = ipGainSchedule;
  
  while (target > pGainScheduleItem->maxValueInclusive)
    pGainScheduleItem++;
    
  return pGainScheduleItem;
}
//------------------------------------------------------------------------------
void CPIDController::LatchValue(double* pValue, double minValue, double maxValue) {
  if (*pValue < minValue)
    *pValue = minValue;
  else if (*pValue > maxValue)
    *pValue = maxValue;
}
