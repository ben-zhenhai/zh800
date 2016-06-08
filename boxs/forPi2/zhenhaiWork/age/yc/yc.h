//#ifndef ZHDef
//#define ZHDef "../zhenhai.h"
//#include ZHDef
//#endif

#include "../zhenhai.h"
#include "../lcd.h"

//#define EVENTSIZE 10
#define EVENTSIZE 9
#define ZHMAXOUTPUT 10
#define RS232Length 70

enum
{
    GOODCOUNT = 0,
    TOTALCOUNT,
    HIGHCAPCOUNT,
    LOWCAPCOUNT,
    DXCOUNT,
    LEAKCOUNT,
    NOLOADCOUNT,
    SHORTCOUNT,
    CONTRARYCOUNT
    //NOAGECOUNT
};

typedef struct RECEIVEPACK
{
    char addr;
    char cmd;
    char length;
    char checksum;
    char capHiDecimal;
    char capLowDecimal;
    char capHiDecimal_2;
    char capLowDecimal_2;
    char currHiDecimal;
    char currLowDecimal;
    char dHiDecimal;
    char currDecimal_1;
    char currDecimal_2;
    char fetState;
    char capDecimal;
    char dDecimal;
    int capHi;
    int capLow;
    int capHi_2;
    int capLow_2;
    int currHi;
    int currLow;
    int dHi;
    int capOver;
    int capLess;
    int damageOver;
    int leakage;
    int noLoad;
    int shortCircuit;
    int contrary;
    int adValue1;
    int adValue2;
    int capValue;
    int dValue;
    int noTest;
    int noUser;
    long totalCount;
    long capPass;
    long capPass_2;
    
} ReceivePack;

//int WriteFile(int mode);

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

int CheckSum();
int SetFrequencyFunction(char value);
float ShiftDecimalFunction(int value, int decimal);
float ShiftDFDecimalFunction(int vluae, int decimal);

