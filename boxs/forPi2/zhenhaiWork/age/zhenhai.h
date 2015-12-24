#ifndef include_ZHENHAI
#define include_ZHENHAI 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>  //wait();
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <wiringPi.h>
#include <wiringSerial.h>
#include <wiringPiSPI.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h> //signal

//#define BarcodeFilePath "/home/pi/works/tsw100/new_barcode"
//#define BarcodeFilePath "/home/pi/zhenhaiWork/age/newbarcode"
#define BarcodeFilePath "/home/pi/works/machineNumber"


#define INPUTLENGTH 256
#define USLEEPTIMEUNITVERYSHORT 1000
#define USLEEPTIMEUNITSHORT 100000
#define USLEEPTIMEUNITMID 500000
#define USLEEPTIMEUNITLONG 700000
//#define ZHNETWORKTYPE "wlan0"
#define ZHNETWORKTYPE "eth0"
#define I2CDEVICEADDR "/dev/i2c-1"
#define Debug

#define I2CIOEXTEND1 0x20
#define I2CIOEXTEND2 0x24
#define I2CIOEXTEND3 0x26

#define INP0 0x00
#define INP1 0x01
#define OUTP0 0x02
#define OUTP1 0x03
#define INVP0 0x04
#define INVP1 0x05
#define CONFIGP0 0x06
#define CONFIGP1 0x07

#define ZHPIN11 0
#define ZHPIN12 1
#define ZHPIN13 2
#define ZHPIN15 3
#define ZHPIN16 4
#define ZHPIN18 5
#define ZHPIN22 6
#define ZHPIN7  7
#define ZHPIN24 10
#define ZHPIN29 21
#define ZHPIN31 22
#define ZHPIN32 26
#define ZHPIN36 27
#define ZHPIN38 28
#define ZHPIN40 29

typedef struct INPUTNODE InputNode;

// 0: timeout leave
// 1: normal leave
// 2: force leave
// 3: change leave
enum
{
    ZHTimeoutExitEvent = 0,
    ZHNormalExitEvent,
    ZHForceExitEvent,
    ZHChangeUserExitEvent,
    ZHPowerOffEvent
};

enum
{
    MachRUNNING = 1,
    MachREPAIRING,
    MachREPAIRDone,
    MachJobDone,
    MachLOCK,
    MachUNLOCK,
    MachSTOPForce1,
    MachSTOPForce2,
    MachSTART,
    MachSTANDBY,
    MachPOWEROFF,
    MachRESUMEFROMPOWEROFF,
    MachREPAIRING2
};

struct INPUTNODE
{
    char ISNo[INPUTLENGTH];
    char ManagerCard[INPUTLENGTH];
    char UserNo[INPUTLENGTH];
    char CountNo[INPUTLENGTH];
    char UploadFilePath[INPUTLENGTH];
    char GoodNo[INPUTLENGTH]; 
    InputNode *link;
};

InputNode *ZHList;
InputNode *ZHNode;
//char FixItemNo[INPUTLENGTH];
//char RepairNo[INPUTLENGTH];
char MachineNo[INPUTLENGTH]; 
int OrderInBox;
int InputDone;
//globle value

pthread_cond_t CondWatchdog, CondMain;
pthread_mutex_t MutexInput, MutexFile, MutexWatchdog, MutexMain, MutexEEPROM, MutexLinklist, MutexScreen;

unsigned long GoodCount;
unsigned long TotalBadCount;

//globle flag
unsigned char UploadFileThreadFlag;
unsigned char WatchdogThreadFlag;
unsigned char WatchdogResetFlag;
unsigned char WatchdogCoolDownCount;
unsigned char InputThreadFlag;
unsigned char ZHResetFlag;
unsigned char SerialFuntionFlag;
unsigned char StopUpdateNetworkStatusFlag;
unsigned char LoopLeaveEventIndex;
unsigned char ButtonEnableFlag;
#endif
