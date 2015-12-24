#ifndef ZHDef
#define ZHDef "../zhenhai.h"
#include ZHDef
#endif

#include <termios.h>
#include "../lcd.h"

#define EVENTSIZE 25
#define WATCHDOGVALUE 1200000
#define WATCHDOGPERIOD 100
#define WRITEFILECOUNTVALUE 4000
#define GOODCOUNT 0
#define ERRORCHECKMAXRETRY 20
#define RS232LENGTH 5
#define ERRORTYPE 22

enum
{
    GoodNumber = 0,
    BadNumber,
    GoodTotalNumber
};
char FileOutput[RS232LENGTH];

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];

pthread_mutex_t MutexSerial;

//global flag
unsigned char SerialFunctionFlag;
unsigned char UpdateFlag;

int WriteFile(int mode);
int SetI2cConfig();
void * ZHSerialFunction(void *argument);
void * WatchdogFunction(void *argument);

unsigned long TransferFormatLongFunction(unsigned char x);
