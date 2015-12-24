//#ifndef ZHDef
//#define ZHDef "../zhenhai.h"
//#include ZHDef
//#endif

#include "../zhenhai.h"
#include "../lcd.h"

#define EVENTSIZE 5
#define ZHMAXOUTPUT 10
#define RS232Length 55
#define WRITEFILETIMER 4000
#define WATCHDOGPERIOD 100

enum
{
    GOODCOUNT = 0,
    CXBAD,
    DXBAD,
    LCBAD,
    TOTAL
};

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];

char FileOutput[RS232Length];

pthread_mutex_t MutexSerial;

//global flag
unsigned char SerialFunctionFlag;
unsigned char UpdateFlag;

int WriteFile(int mode);

int SetI2cConfig();
void * ZHSerialFunction(void *argument);
void * WatchdogFunction(void *argument);
double CXResult(char CX1, char CX2, char CX3, char CX4, char CX5, char CX6);
float DXResult(char DS1, char DS2, char DS3, char DS4, char DS5);
float LCResult(char LC1, char LC2, char LC3, char LC4, char LC5);

float LCSetter(char LC1, char LC2, char LC3, char LC4);
double CXSetter(char CX1, char CX2, char CX3);
double CXUpBoundSetter(char CX1, char CX2, char CX3);
double CXLowBoundSetter(char CX1, char CX2, char CX3);
float DXSetter(char DX1, char DX2, char DX3);

