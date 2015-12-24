//#ifndef ZHDef
//#define ZHDef "../zhenhai.h"
//#include ZHDef
//#endif

#include "../zhenhai.h"
#include <termios.h>
#include "../lcd.h"

#define EVENTSIZE 10
#define WATCHDOGVALUE 1200000
#define WATCHDOGPERIOD 300
#define WRITEFILECOUNTVALUE 4200
#define GOODCOUNT 0
#define ERRORCHECKMAXRETRY 4
#define RS232LENGTH 16
//#define ERRORTYPE 9

enum
{
    GoodNumber = 0,
    BadNumber,
};
char FileOutput[RS232LENGTH];

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];

unsigned int MessageArray;
unsigned int ExMessageArray;

pthread_mutex_t MutexSerial;

//global flag
unsigned char SerialFunctionFlag;
unsigned char UpdateFlag;

int WriteFile(int mode);
int SetI2cConfig();
void * ZHSerialFunction(void *argument);
void * WatchdogFunction(void *argument);

unsigned long TransferFormatLongFunction(unsigned char x);
unsigned int TransferFormatIntFunction(unsigned char x);
