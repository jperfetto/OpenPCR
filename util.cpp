#include "pcr_includes.h"

void sprintFloat(char* str, float val, int decimalDigits) {
  int number = (int)val;
  int decimal = (val - number) * pow(10, decimalDigits);
  sprintf(str, "%3d.%d", number, decimal);
}

void* operator new(size_t size) {
  return malloc(size);
}

void operator delete(void * ptr) {
  free(ptr);
}

void __cxa_pure_virtual(void) {};
