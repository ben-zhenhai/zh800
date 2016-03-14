//#ifndef ZHDef
//#define ZHDef "../zhenhai.h"
//#include ZHDef
//#endif


#include "../zhenhai.h"
#include <termios.h>
#include "../lcd.h"

#define EVENTSIZE 35
#define WATCHDOGVALUE 1200000
#define WATCHDOGPERIOD 300
#define WRITEFILECOUNTVALUE 4200
#define GOODCOUNT 0
#define ERRORCHECKMAXRETRY 4
#define RS232LENGTH 41
//#define ERRORTYPE 9

enum
{
    BadLayout1 = 0,
    BadLayout2,
    BadLayout3,
    BadLayout4,
};

char FileOutput[RS232LENGTH];

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];

unsigned int MessageArray[4];
unsigned int ExMessageArray[4];

pthread_mutex_t MutexSerial;

//global flag
unsigned char SerialFunctionFlag;
unsigned char UpdateFlag;

//int WriteFile(int mode);
int SetI2cConfig();
void * ZHSerialFunction(void *argument);
void * WatchdogFunction(void *argument);

unsigned long TransferFormatLongFunction(unsigned char x);
unsigned int TransferFormatIntFunction(unsigned char x);
