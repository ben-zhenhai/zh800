//#ifndef ZHDef
//#define ZHDef "../zhenhai.h"
//#include ZHDef
//#endif

#include "../zhenhai.h"
#include <termios.h>
#include "../lcd.h"

#define EVENTSIZE 40
#define WATCHDOGVALUE 1200
#define WATCHDOGPERIOD 4
#define WRITEFILECOUNTVALUE 4

#define GOODCOUNT 1
#define ZHMAXOUTPUT 10
#define ERRORCHECKMAXRETRY 3

#define RS232LengthSender 18
#define RS232LengthReceiver 256
#define ErrorType 22

enum
{
    GoodTotalNumber = 0,
    GoodNumber,
    HalfNumner,
    InsertNumber,
    BadTotalNumber
};
pthread_mutex_t MutexSerial;
pthread_cond_t CondSerial;

unsigned long Count[EVENTSIZE];
unsigned long ExCount[EVENTSIZE];
unsigned char NewDataIncome; //for check have new data or not
//global flag
unsigned char SerialFunctionFlag;

//int WriteFile(int mode);
int SetI2cConfig();
void * ZHSerialFunction(void *argument);
unsigned long TransferFormatLongFunction(unsigned char x);
