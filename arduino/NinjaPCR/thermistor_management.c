//LID_THERMISTOR_TABLE_ORIGINAL
//LID_THERMISTOR_TABLE_NXFT15XH103FA
//LID_THERMISTOR_TABLE_CONSTB_3980
//THERMISTOR_TABLE_OPENPCR
//THERMISTOR_TABLE_NO1
//THERMISTOR_TABLE_NO3
//THERMISTOR_TABLE_NO4
//THERMISTOR_TABLE_NO5
//THERMISTOR_TABLE_NO6
//THERMISTOR_TABLE_NO7
//THERMISTOR_TABLE_NO8
//THERMISTOR_TABLE_NO9
//THERMISTOR_TABLE_NXFT15WF104FA2B100

#include <stdio.h>
#include "thermistor_table.h"

#define PLATE_RESISTANCE_TABLE_OFFSET -10
#define TEMP_STEP 1.0

float TableLookup(const unsigned long* lookupTable, unsigned int tableSize, int startValue, unsigned long searchValue);

const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_OPENPCR;

int main(int argc, char **argv) {

  long i = 0;
  double target_temp = 0;

  for (i=5000000; i>0; i--) {
    float temp = TableLookup(PLATE_RESISTANCE_TABLE, sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]), PLATE_RESISTANCE_TABLE_OFFSET, i);
    if (temp > target_temp) {
      printf("%ld\t%3.1f\t%f\n",i, target_temp, temp);
      while (temp>target_temp) {
	target_temp += TEMP_STEP;
      }
    }
  }

}

float TableLookup(const unsigned long* lookupTable, unsigned int tableSize, int startValue, unsigned long searchValue) {
  //simple linear search for now
  int i;
  for (i = 0; i < tableSize; i++) {
    if (searchValue >= lookupTable[i])
      break;
  }

  if (i > 0) {
    unsigned long high_val = lookupTable[i-1];
    unsigned long low_val = lookupTable[i];
    return i + startValue - (float)(searchValue - low_val) / (float)(high_val - low_val);
  } else {
    return startValue;
  }
}
