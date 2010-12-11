#include "pcr_includes.h"

void sprintFloat(char* str, float val, int decimalDigits) {
  int number = (int)val;
  int decimal = (abs(val) - abs(number)) * pow(10, decimalDigits);
  sprintf(str, "%3d.%d", number, decimal);
}

void* operator new(size_t size) {
  return malloc(size);
}

void operator delete(void * ptr) {
  free(ptr);
}

void __cxa_pure_virtual(void) {};

unsigned short htons(unsigned short val) {
  return val << 8 + (byte)val;
}

double UpdatePID(SPid * pid, double error, double position) {
  double pTerm, dTerm, iTerm;
  pTerm = pid->pGain * error; 
  // calculate the proportional term
  // calculate the integral state with appropriate limiting
  pid->iState += error;
  
  if (pid->iState > pid->iMax)
    pid->iState = pid->iMax;
  else if (pid->iState < pid->iMin)
    pid->iState = pid->iMin;
  
  iTerm = pid->iGain * pid->iState; // calculate the integral term
  dTerm = pid->dGain * (position - pid->dState);
  pid->dState = position;
  
  //char buf[255];
  //sprintFloat(buf, pid->iState, 2);
  //Serial.println(buf);
  
  return pTerm + iTerm - dTerm;
}

