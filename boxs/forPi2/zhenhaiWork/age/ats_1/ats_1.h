//#ifndef ZHDef
//#define ZHDef "../zhenhai.h"
//#include ZHDef
//#endif

#include "../zhenhai.h"
#include "../lcd.h"

#define EVENTSIZE 16
#define WATCHDOGPERIOD 4
#define GOODCOUNT 15
#define ZHMAXOUTPUT 10

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];

int I2CEXValue[6];
short CutRoll[2];

//int WriteFile(int mode);

int SetI2cConfig();
void * ZHI2cReaderFunction1(void *argument);
void * WatchdogFunction(void *argument);
