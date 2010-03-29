#include "pcr_includes.h"
#include "thermocycler.h"

#include "display.h"
#include "program.h"

//constants
const unsigned long Thermocycler::PLATE_RESISTANCE_TABLE[] = {
  146735, 138447, 130677, 123390, 116554, 110138, 104113,
  98454, 93137, 88138, 83438, 79016, 74855, 70938, 67249, 63773, 60498, 57410, 54498, 51750, 49157, 46709,
  44397, 42213, 40150, 38199, 36354, 34608, 32957, 31394, 29914, 28512, 27183, 25925, 24731, 23600, 22526,
  21508, 20541, 19623, 18751, 17923, 17136, 16388, 15676, 15000, 14356, 13744, 13161, 12606, 12078, 11574,
  11095, 10637, 10202, 9786, 9389, 9011, 8650, 8306, 7976, 7662, 7362, 7075, 6801, 6539, 6289, 6049, 5820,
  5600, 5391, 5190, 4997, 4813, 4637, 4467, 4305, 4150, 4001, 3858, 3721, 3590, 3464, 3343, 3227, 3115,
  3008, 2905, 2806, 2711, 2620, 2532, 2448, 2367, 2288, 2213, 2141, 2072, 2005, 1940, 1878, 1818, 1761,
  1705, 1652, 1601, 1551, 1503, 1457, 1412, 1369, 1328, 1288, 1250, 1212, 1176, 1142, 1108, 1076, 1045,
  1014 };
    
//public
Thermocycler::Thermocycler():
  ipDisplay(NULL),
  ipProgram(NULL),
  iProgramState(EStopped),
  iThermalState(EHolding),
  ipCurrentStep(NULL),
  iThermalDirection(OFF),
  iPeltierPwm(0) {
    
  ipDisplay = new Display(*this);
  
  //init pins
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
}

Thermocycler::~Thermocycler() {
  delete ipDisplay;
}

// control
void Thermocycler::SetProgram(Cycle* pProgram, Cycle* pDisplayCycle) {
  Stop();
  
  delete ipProgram;
  ipProgram = pProgram;
  ipDisplay->SetDisplayCycle(pDisplayCycle);
}

void Thermocycler::Stop() {
  iProgramState = EStopped;
}

PcrStatus Thermocycler::Start() {
  if (ipProgram == NULL)
    return ENoProgram;
  
  iProgramState = ERunning;
  iThermalState = EHolding;
  iThermalDirection = OFF;
  iPeltierPwm = 0;
  
  ipProgram->BeginIteration();
  ipCurrentStep = ipProgram->GetNextStep();
  return ESuccess;
}

// internal
void Thermocycler::Loop() {
  ReadPlateTemp();
  ControlPeltier();
  ipDisplay->Update();  
}

//private
void Thermocycler::ReadPlateTemp() {
  float voltage = analogRead(PLATE_TEMP_SENSOR_PIN) * 5.0 / 1024;
  int r1 = 4670;
  float resistance = (r1 * 5.0 / voltage) - r1; //(75000 / voltage) - 15000;
 
  //simple linear search for now
  int i;
  for (i = 0; i < sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]); i++) {
    if (resistance >= PLATE_RESISTANCE_TABLE[i])
      break;
  }
  unsigned long high_res = PLATE_RESISTANCE_TABLE[i-1];
  unsigned long low_res = PLATE_RESISTANCE_TABLE[i];
  iPlateTemp = i - 20 - (resistance - low_res) / (high_res - low_res);
}

void Thermocycler::ControlPeltier() {
  float targetTemp = 99;
  ThermalDirection newDirection;
  int newPwm;
  
  //find new direction
  if (iPlateTemp > targetTemp)
    newDirection = COOL;
  else if (iPlateTemp < targetTemp)
    newDirection = HEAT;
  else
    newDirection = OFF;
  
  //find new power
  float tempDiff = abs(targetTemp - iPlateTemp);
  if (tempDiff <= 0.1)
    newPwm = 0;
  else if (tempDiff > 2.0)
    newPwm = 255;
  else
    newPwm = tempDiff * 255 / 2.0;
    
  //change power/direction slowly
  if (newDirection != iThermalDirection)
    newPwm = 0;
  if (newPwm > iPeltierPwm + 2)
    newPwm = iPeltierPwm + 2;
    
  //if (newDirection == COOL && newPwm > 200)
    //newPwm = 200;
    
  //update state
  iPeltierPwm = newPwm;
  iThermalDirection = newDirection;
  SetPeltier(newDirection, newPwm);
}

void Thermocycler::SetPeltier(ThermalDirection dir, int pwm) {
  if (dir == COOL) {
    digitalWrite(2, LOW);
    digitalWrite(4, HIGH);
  } else if (dir == HEAT) {
    digitalWrite(2, HIGH);
    digitalWrite(4, LOW);
  } else {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
  }
  
  analogWrite(3, pwm);
}
