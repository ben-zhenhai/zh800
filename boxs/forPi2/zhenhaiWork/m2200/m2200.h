//#ifndef ZHDef
//#define ZHDef "../zhenhai.h"
//#include ZHDef
//#endif

#include "../zhenhai.h"
#include "../lcd.h"

#define EVENTSIZE 8
#define WATCHDOGVALUE 1200
#define WATCHDOGPERIOD 4
#define GOODCOUNT 5
#define ZHMAXOUTPUT 10

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];

int I2CEXValue[6];

int WriteFile(int mode);

int SetI2cConfig();
void * ZHI2cReaderFunction1(void *argument);
void * WatchdogFunction(void *argument);
