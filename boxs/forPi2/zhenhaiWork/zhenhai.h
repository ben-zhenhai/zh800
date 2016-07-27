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
#include <wiringPiI2C.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h> //signal

//#define BarcodeFilePath "/home/pi/works/tsw100/new_barcode"
//#define BarcodeFilePath "/home/pi/zhenhaiWork/newbarcode"
#define BarcodeFilePath "/home/pi/works/machineNumber"
#define ZHCHECKSCREENBUSY
//#define ZHREFRESHSCREEN

#define INPUTLENGTH 256
#define USLEEPTIMEUNITVERYSHORT 1000
#define USLEEPTIMEUNITSHORT 100000
#define USLEEPTIMEUNITMID 500000
#define USLEEPTIMEUNITLONG 700000

#define VERSION "2016063001"

#define NANOSLEEPTIMEUNITMID 500000000L
#define NANOSLEEPTIMEUNITLONG 700000000L
#define NANOSLEEPTIMEUNITSHORT 100000000L
#define NANOSLEEPTIMEUNITVERYSHORT 1000000L

//#define ZHNETWORKTYPE "wlan0"
#define ZHNETWORKTYPE "eth0"
#define I2CDEVICEADDR "/dev/i2c-1"

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
#define ZHPIN29 21
#define ZHPIN31 22
#define ZHPIN32 26
#define ZHPIN33 23 
#define ZHPIN36 27
#define ZHPIN37 25
#define ZHPIN38 28
#define ZHPIN40 29

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

//globle value
char ISNo[INPUTLENGTH];
char ManagerCard[INPUTLENGTH];
char MachineNo[INPUTLENGTH];
char UserNo[INPUTLENGTH];
char CountNo[INPUTLENGTH];
char GoodNo[INPUTLENGTH];
char FixItemNo[INPUTLENGTH];
char RepairNo[INPUTLENGTH];
char UploadFilePath[INPUTLENGTH];

pthread_cond_t CondWatchdog, CondMain, CondLcdRefresh;
pthread_mutex_t MutexInput, MutexFile, MutexWatchdog, MutexMain, MutexEEPROM, MutexScreen, MutexLcdRefresh;

//globle flag
unsigned char InputDone;
unsigned char UploadFileThreadFlag;
unsigned char WatchdogThreadFlag;
unsigned char WatchdogResetFlag;
unsigned char LcdRefreshFlag;
unsigned char WatchdogCoolDownCount;
unsigned char BarcodeIndex;
unsigned char UploadFileFlag;
unsigned char ZHResetFlag;
unsigned char SerialFuntionFlag;

unsigned char LoopLeaveEventIndex;

unsigned char CanChangeRepairModeFlag;
// Can not into         : 0
// Can into             : 1
// in the repair mode   : 2
// Can leave            : 3
// leave                : 4

unsigned long GoodCount;
unsigned long TotalBadCount;
#endif
