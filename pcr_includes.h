#ifndef _PCR_INCLUDES_H_
#define _PCR_INCLUDES_H_

#include "WProgram.h"

//fixes for incomplete C++ implementation, defined in util.cpp
void* operator new(size_t size);
void operator delete(void * ptr);
void sprintFloat(char* str, float val, int decimalDigits);
extern "C" void __cxa_pure_virtual(void);

//defines
#define STEP_NAME_LENGTH 16
#define MAX_CYCLE_ITEMS 16

enum PcrStatus {
  ESuccess = 0,
  ETooManySteps = 32,
  ENoProgram
};

#define SUCCEEDED(status) (status == ESuccess)

#endif
