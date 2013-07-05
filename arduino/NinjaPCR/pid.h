/*
 *  pid.h - OpenPCR control software.
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

struct SPIDTuning {
  int maxValueInclusive;
  double kP;
  double kI;
  double kD;
};

////////////////////////////////////////////////////////////////////
// Class CPIDController
class CPIDController {
public:
  CPIDController(const SPIDTuning* pGainSchedule, int minOutput, int maxOutput);
 
  //accessors
  double GetIntegrator() { return iIntegrator; }
 
  //computation
  double Compute(double target, double currentValue);

private:
  const SPIDTuning* DetermineGainSchedule(double target);
  void LatchValue(double* pValue, double minValue, double maxValue);
  
private:
  const SPIDTuning* ipGainSchedule;
  int iMinOutput, iMaxOutput;
  double iPreviousError;
  double iIntegrator;
};
