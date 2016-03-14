#ifndef ZHDef
#define ZHDef "../zhenhai.h"
#include ZHDef
#endif

#include "../taicon.h"

#define EVENTSIZE 8
#define WATCHDOGVALUE 1200
#define WATCHDOGPERIOD 4
#define GOODCOUNT 7
#define ZHMAXOUTPUT 10

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];

int I2CEXValue[6];
//short CutRoll[2];

//int WriteFile(int mode);

int SetI2cConfig();
void * ZHI2cReaderFunction1(void *argument);
//void * ZHI2cReaderFunction2(void *argument);
//void * ZHI2cReaderFunction3(void *argument);
void * WatchdogFunction(void *argument);
