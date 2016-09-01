//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"


/* 20160829, Joey Ni, test for LCD bug ------------------ { */
#if 0  // old code
#define CSCHANNEL 0
//#define SPICLOCK 1000
#define SPICLOCK 100000
#endif

#define SPI_CHANNEL 0
#define SPI_SPEED 500000  // Hz
#define CSCHANNEL SPI_CHANNEL
#define SPICLOCK CSCHANNEL
/* 20160829, Joey Ni, test for LCD bug ------------------ } */


int ScreenIndex;
//only in Menu can use button up and down
char DisableUpDown;
char isInPairMode;

void * ChangeScreenEventListenFunction(void *argument);
int UpdateScreenFunction(int screenIndex);
int SendCommandMessageFunction(unsigned char *massage, int arrayLength);
